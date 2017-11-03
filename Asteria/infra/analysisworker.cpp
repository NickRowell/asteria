#include "analysisworker.h"
#include "util/timeutil.h"
#include "infra/analysisinventory.h"

#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

/**
 * TODO:
 *  - For interlaced scan images, compute two localisations per frame corresponding to the odd and even rows
 *  - Implement finer localisation algorithms
 */

AnalysisWorker::AnalysisWorker(QObject *parent, AsteriaState * state, std::vector<std::shared_ptr<Imageuc>> eventFrames)
    : QObject(parent), state(state), eventFrames(eventFrames) {

}

AnalysisWorker::~AnalysisWorker() {
}

void AnalysisWorker::process() {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //               Perform image analysis                  //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Steps:
    // 1) Detect thresholded changed pixels from one image to the next
    // 2) Rough localisation based on changed pixels, maybe median and 3*MAD to place a box around the meteor
    // 3) Precise localisation by centre of flux within the box region
    // 4) Later: best localisation by PSF fitting to centre of flux

    // Only frames that cover the meteor event can be processed; need to apply some threshold
    // at the early stage that rules out an image from being used in the analysis.

    // For each frame that can be processed, localise the meteor; if these are interlaced
    // scan then there are two localisations applied to the odd and even pixels separately.

    // The location in each frame and the time of the frame is used to compute the model fit.

    // How to detect non-meteors?
    //  - dark blob on bright background
    //  - shape of individual images not elliptical etc
    //  - path deviates from model fit
    //  -

    // Initialise an AnalysisInventory with the raw data
    AnalysisInventory inv(eventFrames);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                         //
    // Coarse localisation: 90th percentiles of changed pixels //
    //                                                         //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Note that this is a combination of pixels that got brighter (that the meteor moved into)
    // and pixels that got darker (that the meteor moved out of).

    for(unsigned int i = 1; i < eventFrames.size(); ++i) {

        // Get the current frame and the previous one
        Imageuc &prev = *eventFrames[i-1];
        Imageuc &image = *eventFrames[i];

        // X and Y coordinates of significantly changed pixels
        std::vector<unsigned int> xs;
        std::vector<unsigned int> ys;

        for(unsigned int p=0; p< state->width * state->height; p++) {
            unsigned char newPixel = image.rawImage[p];
            unsigned char oldPixel = prev.rawImage[p];
            if((unsigned int)abs(newPixel - oldPixel) > state->pixel_difference_threshold) {

                xs.push_back(p % state->width);
                ys.push_back(p / state->width);

                if(newPixel - oldPixel > 0) {
                    inv.locs[i].changedPixelsPositive.push_back(p);
                }
                else {
                    inv.locs[i].changedPixelsNegative.push_back(p);
                }
            }
        }

        if(xs.size() > state->n_changed_pixels_for_trigger) {

            // Event detected! Trigger coarse localisation algorithm.
            // Bounding box defined by 90th percentiles of changed pixels locations.
            inv.locs[i].coarse_localisation_success = true;
            std::sort(xs.begin(), xs.end());
            std::sort(ys.begin(), ys.end());
            unsigned int p5 = xs.size() / 20;
            inv.locs[i].bb_xmin=xs[p5];
            inv.locs[i].bb_xmax=xs[xs.size() - 1 - p5];
            inv.locs[i].bb_ymin=ys[p5];
            inv.locs[i].bb_ymax=ys[ys.size() - 1 - p5];
        }
        else {
            inv.locs[i].coarse_localisation_success = false;
        }
    }


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                                   //
    // Fine localisation: centre of flux of box enclosing changed pixels //
    //                                                                   //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    for(unsigned int i = 1; i < eventFrames.size(); ++i) {

        Imageuc &image = *eventFrames[i];

        if(inv.locs[i].coarse_localisation_success) {
            double sum = 0.0;
            inv.locs[i].x_flux_centroid = 0.0;
            inv.locs[i].y_flux_centroid = 0.0;
            for(double x = inv.locs[i].bb_xmin; x <= inv.locs[i].bb_xmax; x++) {
                for(double y = inv.locs[i].bb_ymin; y <= inv.locs[i].bb_ymax; y++) {
                    unsigned int pIdx = y*image.width + x;
                    unsigned int pixel = image.rawImage[pIdx];
                    sum += pixel;
                    // TODO: do we need the 0.5 offset here?
                    inv.locs[i].x_flux_centroid += (x+0.5)*pixel;
                    inv.locs[i].y_flux_centroid += (y+0.5)*pixel;
                }
            }
            inv.locs[i].x_flux_centroid /= sum;
            inv.locs[i].y_flux_centroid /= sum;
        }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                         //
    //       Finer localisation: PSF fitting along track       //
    //                                                         //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // TODO:
    // Fit a straight line (or low-order polynomial) to the i & j coordinates as a function of time
    // Include outlier rejection
    // Reprocess each image to perform PSF fitting centred on the predicted location at the time of the image
    for(unsigned int i = 0; i < eventFrames.size(); ++i) {

    }


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //            Save analysis results to disk              //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    inv.saveToDir(state->videoDirPath);

    // All done - emit signal
    emit finished(TimeUtil::epochToUtcString(eventFrames[0u]->epochTimeUs));
}

