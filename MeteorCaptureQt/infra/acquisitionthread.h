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

/**
 * @brief The AcquisitionState enum enumerates the possible states of the
 * acquisition thread.
 */
enum AcquisitionState{DETECTING, RECORDING, IDLE};


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
     * @brief detectionHeadBuffer
     * Used to buffer the acquired frames so that we have some footage from before an event.
     * This is the 'detection head' footage.
     */
    RingBuffer<std::shared_ptr<Image>> detectionHeadBuffer;

    /**
     * @brief eventFrames
     * Buffer to store the detection and 'detection tail' footage
     */
    std::vector<std::shared_ptr<Image>> eventFrames;

    /**
     * @brief state
     * The current state of the acquisition thread, which determines what is done with newly
     * acquired frames.
     */
    AcquisitionState acqState;

    // The total number of frames recorded for a given event. This is used to limit the
    // length of clips recorded to a manageable size and avoid very long events (which probably
    // aren't meteors) from overwhelming the system.
    // TODO: any sequences that exceed this could be discarded rather than clamped.
    // TODO: add this parameter to the detection parameters
    unsigned int nRecordedFrames = 0;

    // The number of frames recorded since the last trigger. Usually, there will be
    // multiple triggers during a single event, so we reset this counter to zero on each trigger
    // and terminate the recording when it exceeds the detection tail length.
    unsigned int nFramesSinceLastTrigger = 0;

    /**
     * @brief mutex used to control multithreaded use of instances of this class.
     */
    QMutex mutex;
};

#endif // ACQUISITIONTHREAD_H
