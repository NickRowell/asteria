#include "infra/calibrationworker.h"
#include "util/timeutil.h"
#include "util/fileutil.h"
#include "infra/source.h"
#include "util/sourcedetector.h"
#include "util/renderutil.h"
#include "util/coordinateutil.h"
#include "util/mathutil.h"
#include "infra/calibrationinventory.h"
#include "optics/pinholecamerawithradialdistortion.h"

#include "infra/image.h"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>

#include <Eigen/Dense>

#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

CalibrationWorker::CalibrationWorker(QObject *parent, AsteriaState * state, const CalibrationInventory *initial, std::vector<std::shared_ptr<Imageuc>> calibrationFrames)
    : QObject(parent), state(state), initial(initial), calibrationFrames(calibrationFrames) {

}

CalibrationWorker::~CalibrationWorker() {
}

void CalibrationWorker::process() {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //                 Perform calibration                   //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    fprintf(stderr, "Got %lu frames for calibration\n", calibrationFrames.size());

    // The calibration data is assigned to fields of the CalibrationInventory for storage
    CalibrationInventory calInv(calibrationFrames);
    calInv.epochTimeUs = calibrationFrames.front()->epochTimeUs;

    long long midTimeStamp = (calibrationFrames.front()->epochTimeUs + calibrationFrames.back()->epochTimeUs) >> 1;
    unsigned int width = calibrationFrames.front()->width;
    unsigned int height = calibrationFrames.front()->height;


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //   Estimate the signal, noise and background images    //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // This algorithm estimates the signal and noise images from the stack of calibration frames from the
    // trimmed mean and standard deviations of each pixel. This is preferable over other robust statistics
    // such as the median and MAD for the following reason: since the pixel values are clamped to [0:255] range
    // we cannot have far outliers that serioiusly bias the mean; any outliers will be modest and will be removed
    // by using the trimmed mean. The median is quantized and will not be as accurate as the mean given the limited
    // range of values.

    std::vector<double> signal(width * height);
    std::vector<double> noise(width * height);

    // Loop over the pixels
    for(unsigned int p=0; p<width * height; p++) {

        // Extract the pixel value in each of the calibration frames
        std::vector<double> pixels(calibrationFrames.size());
        for(unsigned int f = 0; f < calibrationFrames.size(); f++) {
            double pixel = static_cast<double>(calibrationFrames[f]->rawImage[p]);
            pixels[f] = pixel;
        }

        // Now compute the trimmed mean & sample standard deviation
        double trimmed_mean = 0.0;
        double trimmed_std = 0.0;
        MathUtil::getTrimmedMeanStd(pixels, trimmed_mean, trimmed_std, 0.05);

        signal[p] = trimmed_mean;
        noise[p] = trimmed_std;
    }

    // Now post-process the signal value to get an estimate of the source-free background level in each pixel
    std::vector<double> background(width * height);

    // Algorithm for background calculation: each pixel is the median value of the pixels surrounding it in
    // a window of some particular width.
    // Sliding window extends out to this many pixels on each side of the central pixel
    int hw = (int)state->bkg_median_filter_half_width;
    for(unsigned int k=0; k<height; k++) {
        for(unsigned int l=0; l<width; l++) {

            // Compute the boundary of the window region
            unsigned int k_min = std::max((int)k - hw, 0);
            unsigned int k_max = std::min((int)k + hw, (int)height);
            unsigned int l_min = std::max((int)l - hw, 0);
            unsigned int l_max = std::min((int)l + hw, (int)width);

            // Pixels within the window
            std::vector<double> pixels;
            for(unsigned int kp=k_min; kp<k_max; kp++) {
                for(unsigned int lp=l_min; lp<l_max; lp++) {
                    unsigned int pixIdx = kp*width + lp;
                    pixels.push_back(signal[pixIdx]);
                }
            }

            // Get the median value in the window
            double median = MathUtil::getMedian(pixels);

            unsigned int pixIdx = k*width + l;
            background[pixIdx] = median;
        }
    }

    calInv.noise = make_shared<Imaged>(width, height);
    calInv.noise->epochTimeUs = midTimeStamp;
    calInv.noise->rawImage = noise;

    calInv.signal = make_shared<Imaged>(width, height);
    calInv.signal->epochTimeUs = midTimeStamp;
    calInv.signal->rawImage = signal;

    calInv.background = make_shared<Imaged>(width, height);
    calInv.background->epochTimeUs = midTimeStamp;
    calInv.background->rawImage = background;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //                Extract observed sources               //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    calInv.sources = SourceDetector::getSources(calInv.signal->rawImage, calInv.background->rawImage, calInv.noise->rawImage,
                                                             width, height, state->source_detection_threshold_sigmas);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //       Project the reference stars into the image      //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Get the transformation from BCRF to IM

    // GMST of the calibration frames capture
    double gmst = TimeUtil::epochToGmst(midTimeStamp);

    double lon = MathUtil::toRadians(initial->longitude);
    double lat = MathUtil::toRadians(initial->latitude);

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    Matrix3d r_sez_cam = initial->q_sez_cam.toRotationMatrix();

    // Full transformation BCRF->CAM
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    std::vector<ReferenceStar> visibleReferenceStars;
    for(ReferenceStar &star : state->refStarCatalogue) {

        CoordinateUtil::projectReferenceStar(star, r_bcrf_cam, *initial->cam);

        // Reject stars fainter than faint mag limit
        if(star.mag > state->ref_star_faint_mag_limit) {
            continue;
        }

        if(star.r[2] < 0) {
            // Star is behind the camera
            continue;
        }

        if(star.i>0 && star.i<width && star.j>0 && star.j<height) {
            // Star is visible in image!
            visibleReferenceStars.push_back(star);
        }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //    Cross-match reference stars and observed sources   //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // The cross matching is done purely spatially. The algorithm is as follows:
    // For each Source find the closest ReferenceStar
    // If there are no Sources closer to the ReferenceStar than this one, and the
    // separation is below a threshold, then the Source and ReferenceStar are a match.

    // TODO: make robust to hot pixels. Include a hot pixel determination step that operates
    //       over multiple executions.
    // TODO: allow these to be specified manually somehow; maybe a field of the constructor.

    // Minimum separation for acceptable cross match
    double minSepThreshold = 30.0;
    // Maximum possible separation of two points in the image (diagonal extent of image); used to initialise separation cache
    double maxSep = std::sqrt(width*width + height*height);

    for(unsigned int s1=0; s1<calInv.sources.size(); s1++) {

        Source * source = &(calInv.sources[s1]);
        ReferenceStar * closestStar;
        double minSep = maxSep;

        for(unsigned int s2=0; s2<visibleReferenceStars.size(); s2++) {
            ReferenceStar * testStar = &(visibleReferenceStars[s2]);
            double separation = std::sqrt((source->i - testStar->i)*(source->i - testStar->i) + (source->j - testStar->j)*(source->j - testStar->j));
            if(separation < minSep) {
                minSep = separation;
                closestStar = testStar;
            }
        }

        // If the closest reference star is too far away to be a potential match for this source then skip the source,
        // we can't find a match for it.
        if(minSep > minSepThreshold) {
            continue;
        }

        // Find the closest source to this reference star
        minSep = maxSep;
        Source * closestSource;
        for(unsigned int s2=0; s2<calInv.sources.size(); s2++) {
            Source * testSource = &(calInv.sources[s2]);
            double separation = std::sqrt((testSource->i - closestStar->i)*(testSource->i - closestStar->i) + (testSource->j - closestStar->j)*(testSource->j - closestStar->j));
            if(separation < minSep) {
                minSep = separation;
                closestSource = testSource;
            }
        }

        // If the closest source to this reference star is the original source, then we have a match
        if(closestSource == source) {
            fprintf(stderr, "Found cross-match between Source at %f,%f and ReferenceStar at %f,%f\n", source->i, source->j, closestStar->i, closestStar->j);
            calInv.xms.push_back(pair<Source, ReferenceStar>(*source, *closestStar));
        }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //           Compute the geometric calibration           //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    // TODO: execute the camera calibration algorithm

    CameraModelBase * cam = initial->cam;

    // TODO: enable switching to a different camera model without having to recalibrate from scratch.
    // i.e. here we should detect if the input camera model is different to the one that we want to calibrate, and
    // transform it accordingly.

//    CameraModelBase * cam = new PinholeCameraWithRadialDistortion(state->width, state->height, 300.0, 300.0, 320.0, 240.0, 0.0001, 0.0002, 0.0003, 0.0004, 0.0005);

    // TODO: read out the results from the geometric calibration
    calInv.cam = initial->cam;
    calInv.q_sez_cam = initial->q_sez_cam;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //             Compute the readout noise                 //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // TODO: Measure xrange from percentiles of data
    // TODO: Get readnoise estimate from data
    calInv.readNoiseAdu = 5.0;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //        Copy fixed fields of the calibration           //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    calInv.longitude = initial->longitude;
    calInv.latitude = initial->latitude;
    calInv.altitude = initial->altitude;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //          Save calibration results to disk             //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    calInv.saveToDir(state->calibrationDirPath);

    // All done - emit signal
    emit finished(TimeUtil::epochToUtcString(calibrationFrames[0u]->epochTimeUs));
}
