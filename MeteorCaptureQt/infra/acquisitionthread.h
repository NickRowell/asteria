#ifndef ACQUISITIONTHREAD_H
#define ACQUISITIONTHREAD_H

#include <QThread>
#include <QMutex>
#include "infra/meteorcapturestate.h"

class AcquisitionThread : public QThread
{
    Q_OBJECT

public:
    AcquisitionThread(QObject *parent = 0, MeteorCaptureState * state = 0);
    ~AcquisitionThread();

    void launch();

signals:
    void acquiredImage(char * imageBufferStart);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    MeteorCaptureState * state;
    bool abort;

    QMutex mutex;
};

#endif // ACQUISITIONTHREAD_H
