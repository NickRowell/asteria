#ifndef ANALYSISTHREAD_H
#define ANALYSISTHREAD_H

#include "infra/meteorcapturestate.h"
#include "infra/image.h"

#include <linux/videodev2.h>
#include <vector>               // vector
#include <memory>               // shared_ptr

#include <QThread>
#include <QMutex>

class AnalysisThread : public QThread
{
    Q_OBJECT

public:
    AnalysisThread(QObject *parent = 0, MeteorCaptureState * state = 0, std::vector<std::shared_ptr<Image>> eventFrames = std::vector<std::shared_ptr<Image>>());
    ~AnalysisThread();

    void launch();

signals:

    void done();

protected:

    void run() Q_DECL_OVERRIDE;

private:

    MeteorCaptureState * state;

    bool abort;

    std::vector<std::shared_ptr<Image>> eventFrames;

    QMutex mutex;
};

#endif // ANALYSISTHREAD_H
