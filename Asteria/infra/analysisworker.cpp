#include "analysisworker.h"
#include "util/timeutil.h"
#include "util/ioutil.h"
#include "util/fileutil.h"
#include "util/mathutil.h"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>    // std::max, std::min_element, std::max_element

#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>


// TODO: add a parameter that prevents the frames being written to disk. This would allow us to
//       re-run analyses using existing clips loaded from disk rather than directly from the camera.

AnalysisWorker::AnalysisWorker(QObject *parent, AsteriaState * state, std::vector<std::shared_ptr<Image>> eventFrames)
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

    for(unsigned int i = 0; i < eventFrames.size(); ++i) {
        // Clear any localisation etc fields filled by acquisition thread. We prefer to recompute everything.
        eventFrames[i]->changedPixelsPositive.clear();
        eventFrames[i]->changedPixelsNegative.clear();
    }

    for(unsigned int i = 1; i < eventFrames.size(); ++i) {

        // Get the current frame and the previous one
        Image &prev = *eventFrames[i-1];
        Image &image = *eventFrames[i];
        for(unsigned int p=0; p< state->width * state->height; p++) {
            unsigned char newPixel = image.rawImage[p];
            unsigned char oldPixel = prev.rawImage[p];
            if(abs(newPixel - oldPixel) > state->pixel_difference_threshold) {
                if(newPixel - oldPixel > 0) {
                    image.changedPixelsPositive.push_back(p);
                }
                else {
                    image.changedPixelsNegative.push_back(p);
                }
            }
        }

        if(image.changedPixelsPositive.size() + image.changedPixelsNegative.size() > state->n_changed_pixels_for_trigger) {

            // Event detected! Trigger localisation algorithm(s)

            // Coarse localisation: bounding box defined by median and MAD of changed pixels.
            // Note that this is a combination of pixels that got brighter (that the meteor moved into)
            // and pixels that got darker (that the meteor moved out of).

            // X and Y coordinates of changed pixels
            std::vector<unsigned int> xs;
            std::vector<unsigned int> ys;

            for(unsigned int p = 0; p < image.changedPixelsPositive.size(); ++p) {
                unsigned int pIdx = image.changedPixelsPositive[p];
                unsigned int x = pIdx % state->width;
                unsigned int y = pIdx / state->width;
                xs.push_back(x);
                ys.push_back(y);
            }

            for(unsigned int p = 0; p < image.changedPixelsNegative.size(); ++p) {
                unsigned int pIdx = image.changedPixelsNegative[p];
                unsigned int x = pIdx % state->width;
                unsigned int y = pIdx / state->width;
                xs.push_back(x);
                ys.push_back(y);
            }

            // Coarse localisation: bounding box of changed pixels
//            image.coarse_localisation_success = true;
//            auto xrange = std::minmax_element(xs.begin(), xs.end());
//            auto yrange = std::minmax_element(ys.begin(), ys.end());
//            image.bb_xmin=*xrange.first;
//            image.bb_xmax=*xrange.second;
//            image.bb_ymin=*yrange.first;
//            image.bb_ymax=*yrange.second;

            // Alternatively - robust localisation using median and MAD
//            image.coarse_localisation_success = true;
//            int x_med, x_mad, y_med, y_mad;
//            MathUtil::getMedianMad(xs, x_med, x_mad);
//            MathUtil::getMedianMad(ys, y_med, y_mad);
//            image.bb_xmin=std::max(x_med - 3*x_mad, 0);
//            image.bb_xmax=std::min(x_med + 3*x_mad, (int)state->width-1);
//            image.bb_ymin=std::max(y_med - 3*y_mad, 0);
//            image.bb_ymax=std::min(y_med + 3*y_mad, (int)state->height-1);

            // Alternatively - fit to 90th percentiles of data
            image.coarse_localisation_success = true;
            std::sort(xs.begin(), xs.end());
            std::sort(ys.begin(), ys.end());
            unsigned int p5 = xs.size() / 20;
            image.bb_xmin=xs[p5];
            image.bb_xmax=xs[xs.size() - 1 - p5];
            image.bb_ymin=ys[p5];
            image.bb_ymax=ys[ys.size() - 1 - p5];

            // Finer localisation: centre of flux of pixels within the coarse box
            double sum = 0.0;
            image.x_flux_centroid = 0.0;
            image.y_flux_centroid = 0.0;
            for(double x = image.bb_xmin; x <= image.bb_xmax; x++) {
                for(double y = image.bb_ymin; y <= image.bb_ymax; y++) {
                    unsigned int pIdx = y*image.width + x;
                    unsigned int pixel = image.rawImage[pIdx];
                    sum += pixel;
                    // TODO: do we need the 0.5 offset here?
                    image.x_flux_centroid += (x+0.5)*pixel;
                    image.y_flux_centroid += (y+0.5)*pixel;
                }
            }
            image.x_flux_centroid /= sum;
            image.y_flux_centroid /= sum;

            // TODO: now perform finer localisation

        }
        else {
            image.coarse_localisation_success = false;
        }

    }


    // Create new directory to store results for this clip. The path is set by the
    // date and time of the first frame
    std::string utc = TimeUtil::epochToUtcString(eventFrames[0u]->epochTimeUs);
    std::string yyyy = TimeUtil::extractYearFromUtcString(utc);
    std::string mm = TimeUtil::extractMonthFromUtcString(utc);
    std::string dd = TimeUtil::extractDayFromUtcString(utc);

    std::vector<std::string> subLevels;
    subLevels.push_back(yyyy);
    subLevels.push_back(mm);
    subLevels.push_back(dd);
    subLevels.push_back(utc);
    string path = state->videoDirPath + "/" + yyyy + "/" + mm + "/" + dd + "/" + utc;

    if(!FileUtil::createDirs(state->videoDirPath, subLevels)) {
        fprintf(stderr, "Couldn't create directory %s\n", path.c_str());
        return;
    }

    // Write all the images to file and create a peak hold image
    Image peakHold(state->width, state->height, static_cast<unsigned char>(0));
    peakHold.epochTimeUs = eventFrames[0]->epochTimeUs;

    for(unsigned int i = 0; i < eventFrames.size(); ++i) {

        Image &image = *eventFrames[i];

        // Write the image data out to a file
        char filename [100];
        string utcFrame = TimeUtil::epochToUtcString(image.epochTimeUs);
        sprintf(filename, "%s/%s.pgm", path.c_str(), utcFrame.c_str());

        // PGM (grey image)
        std::ofstream out(filename);
        out << image;
        out.close();

        // Compute peak hold image
        for(unsigned int k=0; k<image.height; k++) {
            for(unsigned int l=0; l<image.width; l++) {
                unsigned int offset = k*image.width + l;
                peakHold.rawImage[offset] = std::max(peakHold.rawImage[offset], image.rawImage[offset]);
            }
        }
    }

    // Write out the peak hold image
    char filename [100];
    string utcFrame = TimeUtil::epochToUtcString(eventFrames[0]->epochTimeUs);
    sprintf(filename, "%s/peakhold_%s.pgm", path.c_str(), utcFrame.c_str());
    std::ofstream out(filename);
    out << peakHold;
    out.close();

    // All done - emit signal
    emit finished(utc);
}

