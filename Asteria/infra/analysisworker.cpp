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

    // Coordinates of changed pixel
    // TODO: verify this - are the pixels packed by row?
//    unsigned int x = p % state->width;
//    unsigned int y = p / state->width;



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

