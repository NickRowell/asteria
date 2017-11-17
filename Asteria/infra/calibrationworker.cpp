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

CalibrationWorker::CalibrationWorker(QObject *parent, AsteriaState * state, std::vector<std::shared_ptr<Imageuc>> calibrationFrames)
    : QObject(parent), state(state), calibrationFrames(calibrationFrames) {

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

    std::vector<double> signal(state->width * state->height);
    std::vector<double> noise(state->width * state->height);

    // Loop over the pixels
    for(unsigned int p=0; p<state->width * state->height; p++) {

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
    std::vector<double> background(state->width * state->height);

    // Algorithm for background calculation: each pixel is the median value of the pixels surrounding it in
    // a window of some particular width.
    // Sliding window extends out to this many pixels on each side of the central pixel
    int hw = (int)state->bkg_median_filter_half_width;
    for(unsigned int k=0; k<state->height; k++) {
        for(unsigned int l=0; l<state->width; l++) {

            // Compute the boundary of the window region
            unsigned int k_min = std::max((int)k - hw, 0);
            unsigned int k_max = std::min((int)k + hw, (int)state->height);
            unsigned int l_min = std::max((int)l - hw, 0);
            unsigned int l_max = std::min((int)l + hw, (int)state->width);

            // Pixels within the window
            std::vector<double> pixels;
            for(unsigned int kp=k_min; kp<k_max; kp++) {
                for(unsigned int lp=l_min; lp<l_max; lp++) {
                    unsigned int pixIdx = kp*state->width + lp;
                    pixels.push_back(signal[pixIdx]);
                }
            }

            // Get the median value in the window
            double median = MathUtil::getMedian(pixels);

            unsigned int pixIdx = k*state->width + l;
            background[pixIdx] = median;
        }
    }


    calInv.noise = make_shared<Imaged>(state->width, state->height);
    calInv.noise->epochTimeUs = midTimeStamp;
    calInv.noise->rawImage = noise;

    calInv.signal = make_shared<Imaged>(state->width, state->height);
    calInv.signal->epochTimeUs = midTimeStamp;
    calInv.signal->rawImage = signal;

    calInv.background = make_shared<Imaged>(state->width, state->height);
    calInv.background->epochTimeUs = midTimeStamp;
    calInv.background->rawImage = background;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //                Extract observed sources               //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    calInv.sources = SourceDetector::getSources(calInv.signal->rawImage, calInv.background->rawImage, calInv.noise->rawImage,
                                                             state->width, state->height, state->source_detection_threshold_sigmas);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //       Project the reference stars into the image      //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // TODO: need an initial guess of the geometric calibration


    // Get the transformation from BCRF to IM

    // GMST of the calibration frames capture
    double gmst = TimeUtil::epochToGmst(midTimeStamp);

    double lon = MathUtil::toRadians(state->longitude);
    double lat = MathUtil::toRadians(state->latitude);
    double az = MathUtil::toRadians(state->azimuth);
    double el = MathUtil::toRadians(state->elevation);
    double roll = MathUtil::toRadians(state->roll);

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    Matrix3d r_sez_cam = CoordinateUtil::getSezToCamRot(az, el, roll);
    Matrix3d r_cam_im = CoordinateUtil::getCamIntrinsicMatrix(state->focal_length, state->pixel_width, state->pixel_height, state->width, state->height);

    // Full transformation
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    std::vector<ReferenceStar> refStarsVisible;
    for(ReferenceStar &star : state->refStarCatalogue) {

        // Unit vector towards star in original frame:
        Vector3d r_bcrf;
        CoordinateUtil::sphericalToCartesian(r_bcrf, 1.0, star.ra, star.dec);
        // Transform to CAM frame:
        Vector3d r_cam = r_bcrf_cam * r_bcrf;

        if(r_cam[2] < 0) {
            // Star is behind the camera
            continue;
        }

        // Project into image coordinates
        Vector3d r_im = r_cam_im * r_cam;

        double i = r_im[0] / r_im[2];
        double j = r_im[1] / r_im[2];

        if(i>0 && i<state->width && j>0 && j<state->height) {
            // Star is visible in image!
//            fprintf(stderr, "%f\t%f\t%f\n", i, j, star.mag);
        }

    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //    Cross-match reference stars and observed sources   //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // TODO: allow these to be specified manually somehow; maybe a field of the constructor?


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //           Compute the geometric calibration           //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

//    CameraModelBase * cam = new PinholeCamera(state->width, state->height, 300.0, 300.0, 320.0, 240.0);

    CameraModelBase * cam = new PinholeCameraWithRadialDistortion(state->width, state->height, 300.0, 300.0, 320.0, 240.0, 0.0001, 0.0002, 0.0003, 0.0004, 0.0005);

    calInv.cam = cam;

    // TODO: Measure xrange from percentiles of data
    // TODO: Get readnoise estimate from data
    calInv.readNoiseAdu = 5.0;

    calInv.q_sez_cam.w() = 0.5;
    calInv.q_sez_cam.x() = -0.5;
    calInv.q_sez_cam.y() = 0.5;
    calInv.q_sez_cam.z() = -0.5;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //          Save calibration results to disk             //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    calInv.saveToDir(state->calibrationDirPath);

    // All done - emit signal
    emit finished(TimeUtil::epochToUtcString(calibrationFrames[0u]->epochTimeUs));
}
