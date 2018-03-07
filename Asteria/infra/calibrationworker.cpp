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
#include "optics/pinholecamerawithsipdistortion.h"
#include "math/geocalfitter.h"

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

    long long midTimeStamp = (calibrationFrames.front()->epochTimeUs + calibrationFrames.back()->epochTimeUs) >> 1;
    unsigned int width = calibrationFrames.front()->width;
    unsigned int height = calibrationFrames.front()->height;

    calInv.epochTimeUs = midTimeStamp;

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

        // Reject stars fainter than faint mag limit
        if(star.mag > state->ref_star_faint_mag_limit) {
            continue;
        }

        CoordinateUtil::projectReferenceStar(star, r_bcrf_cam, *initial->cam);

        if(star.visible) {
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
    // TODO: allow cross-matches to be specified manually somehow; maybe a field of the constructor.

    // Minimum separation for acceptable cross match in sigmas
    double minSepThreshold = 20.0;

    // Compute the covariance-weighted separations of all pairs of sources and reference stars
    double covWeightedSep[calInv.sources.size()][visibleReferenceStars.size()];
    for(unsigned int s1=0; s1<calInv.sources.size(); s1++) {

        Source * source = &(calInv.sources[s1]);
        Matrix2d s;
        s << source->c_ii, source->c_ij, source->c_ij, source->c_jj;

        for(unsigned int s2=0; s2<visibleReferenceStars.size(); s2++) {

            ReferenceStar * testStar = &(visibleReferenceStars[s2]);
            MatrixXd r(2,1);
            r << source->i - testStar->i, source->j - testStar->j;

            covWeightedSep[s1][s2] = std::sqrt((r.transpose() * s.colPivHouseholderQr().solve(r))(0,0));
        }
    }

    for(unsigned int s1=0; s1<calInv.sources.size(); s1++) {

        // Locate the closest reference star to source s1
        unsigned int closestStarIdx;
        double minSep = 2.0 * minSepThreshold;
        for(unsigned int s2=0; s2<visibleReferenceStars.size(); s2++) {
            if(covWeightedSep[s1][s2] < minSep) {
                minSep = covWeightedSep[s1][s2];
                closestStarIdx = s2;
            }
        }

        if(minSep > minSepThreshold) {
            // The closest reference star is too far away to be a positive match
            continue;
        }

        // Find the closest source to this reference star
        minSep = 2.0 * minSepThreshold;
        unsigned int closestSourceIdx;
        for(unsigned int s2=0; s2<calInv.sources.size(); s2++) {
            if(covWeightedSep[s2][closestStarIdx] < minSep) {
                minSep = covWeightedSep[s2][closestStarIdx];
                closestSourceIdx = s2;
            }
        }

        // If the closest source to this reference star is the original source, then we have a match
        if(closestSourceIdx == s1) {
            calInv.xms.push_back(pair<Source, ReferenceStar>(calInv.sources[closestSourceIdx], visibleReferenceStars[closestStarIdx]));
        }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //           Compute the geometric calibration           //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    fprintf(stderr, "Initial camera parameters = \n");
    double camPar[initial->cam->getNumParameters()];
    initial->cam->getParameters(camPar);
    for(unsigned int n=0; n<initial->cam->getNumParameters(); n++) {
        fprintf(stderr, "%.10f\t", camPar[n]);
    }

//    calInv.cam = initial->cam->convertToPinholeCameraWithSipDistortion();
    calInv.cam = initial->cam->convertToPinholeCameraWithRadialDistortion();
//    calInv.cam = initial->cam->convertToPinholeCamera();
//    calInv.cam = initial->cam;

    calInv.q_sez_cam = initial->q_sez_cam;

    fprintf(stderr, "Initial quaternion normalisation = %f\n", calInv.q_sez_cam.norm());
    calInv.q_sez_cam.normalize();

    fprintf(stderr, "Initial parameters = \nIntrinsic = ");
    camPar[calInv.cam->getNumParameters()];
    calInv.cam->getParameters(camPar);
    for(unsigned int n=0; n<calInv.cam->getNumParameters(); n++) {
        fprintf(stderr, "%f\t", camPar[n]);
    }
    fprintf(stderr, "\nExtrinsic = %f\t%f\t%f\t%f\n", calInv.q_sez_cam.w(), calInv.q_sez_cam.x(), calInv.q_sez_cam.y(), calInv.q_sez_cam.z());

    GeoCalFitter fitter(calInv.cam, &(calInv.q_sez_cam), &(calInv.xms), gmst, lon, lat);
    fitter.fit(500, true);

    fprintf(stderr, "Final quaternion normalisation = %f\n", calInv.q_sez_cam.norm());

    fprintf(stderr, "Fitted parameters = \nIntrinsic = ");
    calInv.cam->getParameters(camPar);
    for(unsigned int n=0; n<calInv.cam->getNumParameters(); n++) {
        fprintf(stderr, "%f\t", camPar[n]);
    }
    fprintf(stderr, "\nExtrinsic = %f\t%f\t%f\t%f\n", calInv.q_sez_cam.w(), calInv.q_sez_cam.x(), calInv.q_sez_cam.y(), calInv.q_sez_cam.z());


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
