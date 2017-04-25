#include "analysisworker.h"
#include "util/timeutil.h"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>

#include <QDebug>
#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

AnalysisWorker::AnalysisWorker(QObject *parent, MeteorCaptureState * state, std::vector<std::shared_ptr<Image>> eventFrames)
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

    qInfo() << "Analysis thread; iterating over " << eventFrames.size() << " images";

    // Create new directory to store results for this clip
    string path = state->videoDirPath + "/" + TimeUtil::convertToUtcString(eventFrames[0u]->epochTimeUs);
    int status = mkdir(path.c_str(), S_IRWXU);
    if(status == -1) {
        qInfo() << "Could not create directory " << path.c_str();
        return;
    }

    for(unsigned int i = 0; i < eventFrames.size(); ++i) {

        Image &image = *eventFrames[i];

        // Write the image data out to a file
        int imgFileHandle;
        char filename [100];

        string utc = TimeUtil::convertToUtcString(image.epochTimeUs);

        sprintf(filename, "%s/%s.pgm", path.c_str(), utc.c_str());

        if((imgFileHandle = open(filename, O_WRONLY | O_CREAT, 0660)) < 0){
            perror("open");
            return;
        }

        // PGM (grey image)
        std::ofstream out(filename);
        // Raw PGMs:
        out << "P5\n" << state->width << " " << state->height << " 255\n";
        for(unsigned int k=0; k<state->height; k++) {
            for(unsigned int l=0; l<state->width; l++) {
                unsigned int offset = k*state->width + l;
                unsigned char pix = image.rawImage[offset];
                out << pix;
            }
        }
        out.close();

    }

    // All done - emit signal
    emit finished();
}

