#include "analysisthread.h"

#include <QDebug>
#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

AnalysisThread::AnalysisThread(QObject *parent, MeteorCaptureState * state)
    : QThread(parent)
{
    this->state = state;
    abort = false;

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



}

