#include "analysisworker.h"
#include "util/timeutil.h"
#include "util/ioutil.h"
#include "util/fileutil.h"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>    // std::max

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

    for(unsigned int i = 1; i < eventFrames.size(); ++i) {

        // Get the current frame and the previous one
        Image &prev = *eventFrames[i-1];
        Image &image = *eventFrames[i];

        vector<unsigned int> changedPixels;

        for(unsigned int p=0; p< state->width * state->height; p++) {
            if(abs(image.rawImage[p] - prev.rawImage[p]) > state->pixel_difference_threshold) {
                changedPixels.push_back(p);
            }
        }

        if(changedPixels.size() > state->n_changed_pixels_for_trigger) {
            // Event detected! Trigger localisation algorithm(s)
            image.coarse_localisation_success = true;

            // Get borders of square enclosing the changed pixels. Origin is at top left of image.
            image.bb_xmin=state->width;
            image.bb_xmax=0;
            image.bb_ymin=state->height;
            image.bb_ymax=0;
            for(unsigned int p = 0; p < changedPixels.size(); ++p) {

                // Get the pixel index
                unsigned int pIdx = changedPixels[p];
                unsigned int x = pIdx % state->width;
                unsigned int y = pIdx / state->width;

                image.bb_xmin = std::min(x, image.bb_xmin);
                image.bb_xmax = std::max(x, image.bb_xmax);
                image.bb_ymin = std::min(y, image.bb_ymin);
                image.bb_ymax = std::max(y, image.bb_ymax);
            }
        }
        else {
            image.coarse_localisation_success = false;
        }

    }


















    // Create new directory to store results for this clip. The path is set by the
    // date and time of the first frame
    std::string utc = TimeUtil::convertToUtcString(eventFrames[0u]->epochTimeUs);
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
        string utcFrame = TimeUtil::convertToUtcString(image.epochTimeUs);
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
    string utcFrame = TimeUtil::convertToUtcString(eventFrames[0]->epochTimeUs);
    sprintf(filename, "%s/peakhold_%s.pgm", path.c_str(), utcFrame.c_str());
    std::ofstream out(filename);
    out << peakHold;
    out.close();

    // All done - emit signal
    emit finished(utc);
}

