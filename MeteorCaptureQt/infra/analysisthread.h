#ifndef ANALYSISTHREAD_H
#define ANALYSISTHREAD_H

#include <linux/videodev2.h>
#include <vector>
#include <memory>               // shared_ptr

#include <QThread>
#include <QMutex>
#include "infra/meteorcapturestate.h"
#include "infra/image.h"

class AnalysisThread : public QThread
{
    Q_OBJECT

public:
    AnalysisThread(QObject *parent = 0, MeteorCaptureState * state = 0);
    ~AnalysisThread();

    void launch();

signals:
//    void acquiredImage(std::shared_ptr<Image>);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    MeteorCaptureState * state;
    bool abort;

    QMutex mutex;
};

#endif // ANALYSISTHREAD_H
