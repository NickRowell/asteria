#ifndef ACQUISITIONTHREAD_H
#define ACQUISITIONTHREAD_H

#include <linux/videodev2.h>
#include <vector>
#include <memory>               // shared_ptr

#include <QThread>
#include <QMutex>

#include "infra/meteorcapturestate.h"
#include "infra/image.h"
#include "infra/ringbuffer.h"

class AcquisitionThread : public QThread
{
    Q_OBJECT

public:
    AcquisitionThread(QObject *parent = 0, MeteorCaptureState * state = 0);
    ~AcquisitionThread();

    void launch();

signals:
    void acquiredImage(std::shared_ptr<Image>);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    MeteorCaptureState * state;
    bool abort;

    /**
     * \brief Array of pointers to the start of each image buffer in memory
     */
    unsigned char ** buffer_start;

    /**
     * @brief ringBuffer
     * Used to buffer the acquired frames so that we have some footage from before an event
     */
    RingBuffer ringBuffer;

    QMutex mutex;
};

#endif // ACQUISITIONTHREAD_H
