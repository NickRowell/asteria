#include "analysisworker.h"
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

    std::cout << '\n' << "Analysis thread; iterating over " << eventFrames.size() << " images" << '\n';

    // Create new directory to store results for this clip. The path is set by the
    // date and time of the first frame
    std::string utc = TimeUtil::convertToUtcString(eventFrames[0u]->epochTimeUs);
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

    // Write out the peak hold image
    char filename [100];
    string utcFrame = TimeUtil::convertToUtcString(eventFrames[0]->epochTimeUs);
    sprintf(filename, "%s/peakhold_%s.pgm", path.c_str(), utcFrame.c_str());

    // PGM (grey image)
    std::ofstream out(filename);
    out << peakHold;
    out.close();

    // All done - emit signal
    emit finished(utc);
}

