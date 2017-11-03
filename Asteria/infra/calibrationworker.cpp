#include "infra/calibrationworker.h"
#include "util/timeutil.h"
#include "util/fileutil.h"
#include "infra/source.h"
#include "util/sourcedetector.h"
#include "util/renderutil.h"
#include "util/coordinateutil.h"
#include "util/mathutil.h"
#include "infra/calibrationinventory.h"

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
    unsigned int field = calibrationFrames.front()->field;

    // Compute the median image; for each pixel, store a vector of all the values
    std::vector< std::vector<unsigned char>> pixels(state->width * state->height);

    for(unsigned int i = 0; i < calibrationFrames.size(); ++i) {

        Imageuc &image = *calibrationFrames[i];

        // Accumulate pixel values
        for(unsigned int k=0; k<image.height; k++) {
            for(unsigned int l=0; l<image.width; l++) {
                unsigned int pixIdx = k*image.width + l;
                pixels[pixIdx].push_back(image.rawImage[pixIdx]);
            }
        }
    }

    // Compute the median, MAD and RMS value of each pixel
    std::vector<unsigned char> medianVals;
    std::vector<unsigned char> madVals;
    std::vector<double> meanVals;
    std::vector<double> varianceVals;
    double maxVariance = 0;
    int histogramOfDeviations[512] = {0};

    for(unsigned int k=0; k<state->height; k++) {
        for(unsigned int l=0; l<state->width; l++) {
            unsigned int pixIdx = k*state->width + l;

            vector<unsigned char> pixel = pixels[pixIdx];

            double mean = 0.0;
            double meanOfSquare = 0.0;
            for(unsigned int p = 0; p < pixel.size(); p++) {
                double dPix = (double)pixel[p];
                mean += dPix;
                meanOfSquare += (dPix * dPix);
            }
            mean /= (double)pixel.size();
            meanOfSquare /= (double)pixel.size();
            double variance = meanOfSquare - (mean*mean);
            meanVals.push_back(mean);
            varianceVals.push_back(variance);
            maxVariance = std::max(variance, maxVariance);

            unsigned char med;
            unsigned char mad;
            std::sort(pixel.begin(), pixel.end());

            // Compute the median value
            if(pixel.size() % 2 == 0) {
                // Even number of elements - take average of central two
                unsigned int a = (int)pixel[pixel.size()/2];
                unsigned int b = (int)pixel[pixel.size()/2 - 1];
                unsigned int c = (a + b)/2;
                unsigned char d = c & 0xFF;
                med = d;
            }
            else {
                // Odd number of elements - pick central one
                med = pixel[pixel.size()/2];
            }

            // Compute the MAD value
            std::vector<unsigned int> absDevs;
            for(unsigned int p = 0; p < pixel.size(); ++p) {
                int dev = (int)pixel[p] - med;
                histogramOfDeviations[dev+256]++;
                absDevs.push_back(abs(dev));
            }

            std::sort(absDevs.begin(), absDevs.end());

            if (pixel.size() % 2 == 0) {
                mad = (absDevs[pixel.size() / 2 - 1] + absDevs[pixel.size() / 2]) / 2;
            }
            else {
                mad = absDevs[pixel.size() / 2];
            }

            medianVals.push_back(med);
            madVals.push_back(mad);
        }
    }

    // Construct an image of the noise level in each pixel to detect fixed pattern noise etc
    std::vector<double> stdVals;
    for(unsigned int i=0; i<state->width*state->height; i++) {
        double std = std::sqrt(varianceVals[i]);
        stdVals.push_back(std);
    }

    std::shared_ptr<Imaged> noise = make_shared<Imaged>(state->width, state->height);
    noise->epochTimeUs = midTimeStamp;
    noise->rawImage = stdVals;

    std::shared_ptr<Imageuc> median = make_shared<Imageuc>(state->width, state->height);
    median->field = field;
    median->epochTimeUs = midTimeStamp;
    median->rawImage = medianVals;

    // Measure the background image from the median
    std::shared_ptr<Imageuc> background = make_shared<Imageuc>(state->width, state->height);
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
            std::vector<unsigned char> pixels;
            for(unsigned int kp=k_min; kp<k_max; kp++) {
                for(unsigned int lp=l_min; lp<l_max; lp++) {
                    unsigned int pixIdx = kp*state->width + lp;
                    pixels.push_back(medianVals[pixIdx]);
                }
            }

            // Get the median value in the window
            std::sort(pixels.begin(), pixels.end());
            unsigned char med;
            if(pixels.size() % 2 == 0) {
                // Even number of elements - take average of central two
                unsigned int a = (int)pixels[pixels.size()/2];
                unsigned int b = (int)pixels[pixels.size()/2 - 1];
                unsigned int c = (a + b)/2;
                unsigned char d = c & 0xFF;
                med = d;
            }
            else {
                // Odd number of elements - pick central one
                med = pixels[pixels.size()/2];
            }
            unsigned int pixIdx = k*state->width + l;
            background->rawImage[pixIdx] = med;
        }
    }

    // Set these images to the fields in the CalibrationInventory
    calInv.medianImage = median;
    calInv.noiseImage = noise;
    calInv.backgroundImage = background;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //                Extract observed sources               //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

//    calInv.sources = SourceDetector::getSources(median->rawImage, background->rawImage, noise->rawImage,
//                                                             state->width, state->height, state->source_detection_threshold_sigmas);

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
