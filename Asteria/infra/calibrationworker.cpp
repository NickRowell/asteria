#include "infra/calibrationworker.h"
#include "util/timeutil.h"

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

    std::cout << '\n' << "Calibration thread; iterating over " << calibrationFrames.size() << " images" << '\n';

    // Create new directory to store results for this clip. The path is set by the
    // date and time of the first frame
    std::string utc = TimeUtil::convertToUtcString(calibrationFrames[0u]->epochTimeUs);
    std::string yyyy = TimeUtil::extractYearFromUtcString(utc);
    std::string mm = TimeUtil::extractMonthFromUtcString(utc);
    std::string dd = TimeUtil::extractDayFromUtcString(utc);

    // Create the YYYY directory level if it doesn't already exist
    string yearPath = state->videoDirPath + "/" + yyyy;
    struct stat info;
    if( stat( yearPath.c_str(), &info ) != 0 ) {
        // Path does not exist; create it.
        if(mkdir(yearPath.c_str(), S_IRWXU) == -1) {
            qInfo() << "Could not create directory " << yearPath.c_str();
            return;
        }
    }
    else if( info.st_mode & S_IFDIR )  {
        // Exists and is a directory; take no action
    }
    else if( S_ISREG(info.st_mode)) {
        // Exists and is a regular file.
        qInfo() << "Found a regular file at " << yearPath.c_str() << "; can't save videos!";
        return;
    }
    else {
        // Exists and is neither a directory or regular file
        qInfo() << "Found an existing file of unknown type at " << yearPath.c_str() << "; can't save videos!";
        return;
    }

    // Create the MM directory level if it doesn't already exist
    string monthPath = yearPath + "/" + mm;
    if( stat( monthPath.c_str(), &info ) != 0 ) {
        // Path does not exist; create it.
        if(mkdir(monthPath.c_str(), S_IRWXU) == -1) {
            qInfo() << "Could not create directory " << monthPath.c_str();
            return;
        }
    }
    else if( info.st_mode & S_IFDIR )  {
        // Exists and is a directory; take no action
    }
    else if( S_ISREG(info.st_mode)) {
        // Exists and is a regular file.
        qInfo() << "Found a regular file at " << monthPath.c_str() << "; can't save videos!";
        return;
    }
    else {
        // Exists and is neither a directory or regular file
        qInfo() << "Found an existing file of unknown type at " << monthPath.c_str() << "; can't save videos!";
        return;
    }

    // Create the DD directory level if it doesn't already exist
    string dayPath = monthPath + "/" + dd;
    if( stat( dayPath.c_str(), &info ) != 0 ) {
        // Path does not exist; create it.
        if(mkdir(dayPath.c_str(), S_IRWXU) == -1) {
            qInfo() << "Could not create directory " << dayPath.c_str();
            return;
        }
    }
    else if( info.st_mode & S_IFDIR )  {
        // Exists and is a directory; take no action
    }
    else if( S_ISREG(info.st_mode)) {
        // Exists and is a regular file.
        qInfo() << "Found a regular file at " << dayPath.c_str() << "; can't save videos!";
        return;
    }
    else {
        // Exists and is neither a directory or regular file
        qInfo() << "Found an existing file of unknown type at " << dayPath.c_str() << "; can't save videos!";
        return;
    }

    string path = dayPath + "/" + utc;
    int status = mkdir(path.c_str(), S_IRWXU);
    if(status == -1) {
        qInfo() << "Could not create directory " << path.c_str();
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
//                qInfo() << "a, b, c = " << a << ", " << b << ", " << c;

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

