#include "infra/calibrationworker.h"
#include "util/timeutil.h"
#include "util/fileutil.h"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>    // std::max

#include <QDebug>
#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

CalibrationWorker::CalibrationWorker(QObject *parent, AsteriaState * state, std::vector<std::shared_ptr<Image>> calibrationFrames)
    : QObject(parent), state(state), calibrationFrames(calibrationFrames) {

}

CalibrationWorker::~CalibrationWorker() {
}

void CalibrationWorker::process() {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //               Perform image analysis                  //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    fprintf(stderr, "Got %d frames for calibration\n", calibrationFrames.size());

    // Create new directory to store results for this clip. The path is set by the
    // date and time of the first frame
    std::string utc = TimeUtil::convertToUtcString(calibrationFrames[0u]->epochTimeUs);
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
        qInfo() << "Couldn't create directory " << path.c_str() << "!";
        return;
    }

    // Compute the median image; for each pixel, store a vector of all the values
    vector< vector<unsigned char>> pixels(state->width * state->height);

    for(unsigned int i = 0; i < calibrationFrames.size(); ++i) {

        Image &image = *calibrationFrames[i];

        // Accumulate pixel values
        for(unsigned int k=0; k<image.height; k++) {
            for(unsigned int l=0; l<image.width; l++) {
                unsigned int pixIdx = k*image.width + l;
                pixels[pixIdx].push_back(image.rawImage[pixIdx]);
            }
        }
    }

    // Compute the median value of each pixel
    std::vector<unsigned char> medianVals;
    for(unsigned int k=0; k<state->height; k++) {
        for(unsigned int l=0; l<state->width; l++) {
            unsigned int pixIdx = k*state->width + l;

            // Compute median value of pixels
            vector<unsigned char> pixel = pixels[pixIdx];
            // First sort them into ascending order
            std::sort(pixel.begin(), pixel.end());
            // Now pick the middle value, or the average of the two middle values
            if(pixel.size() % 2 == 0) {
                // Even number of elements - take average of central two
                unsigned int a = (int)pixel[pixel.size()/2];
                unsigned int b = (int)pixel[pixel.size()/2 - 1];
                unsigned int c = (a + b)/2;
                unsigned char d = c & 0xFF;
                medianVals.push_back(d);
            }
            else {
                // Odd number of elements - pick central one
                medianVals.push_back(pixel[pixel.size()/2]);
            }
        }
    }

    Image median(state->width, state->height);
    median.rawImage = medianVals;

    // Write out the median image
    char filename [100];
    string utcFrame = TimeUtil::convertToUtcString(calibrationFrames[0]->epochTimeUs);
    sprintf(filename, "%s/median_%s.pgm", path.c_str(), utcFrame.c_str());

    // PGM (grey image)
    std::ofstream out(filename);
    out << median;
    out.close();

    // All done - emit signal
    emit finished(utc);
}

