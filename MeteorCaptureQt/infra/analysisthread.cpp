#include "analysisthread.h"
#include "util/timeutil.h"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#include <QDebug>
#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

AnalysisThread::AnalysisThread(QObject *parent, MeteorCaptureState * state, std::vector<std::shared_ptr<Image>> eventFrames)
    : QThread(parent), state(state), eventFrames(eventFrames), abort(false) {


}

AnalysisThread::~AnalysisThread()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();

}

void AnalysisThread::launch() {

    // Lock this object for the duration of this function
    QMutexLocker locker(&mutex);

    if (!isRunning()) {
        start(NormalPriority);
    }
}


void AnalysisThread::run() {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //               Perform image analysis                  //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    qInfo() << "Analysis thread; iterating over " << eventFrames.size() << " images";

    for(unsigned int i = 0; i < eventFrames.size(); ++i) {

        Image &image = *eventFrames[i];

        // Write the image data out to a file
        int imgFileHandle;
        char filename [100];

        string utc = TimeUtil::convertToUtcString(image.epochTimeUs);

        sprintf(filename, "/home/nrowell/Temp/%s.pgm", utc.c_str());

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
                unsigned char pix = image.pixelData[offset];
                out << pix;
            }
        }
        out.close();

    }


}

