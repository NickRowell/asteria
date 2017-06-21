#ifndef ACQUISITIONTHREAD_H
#define ACQUISITIONTHREAD_H

#include <linux/videodev2.h>
#include <vector>
#include <memory>               // shared_ptr

#include <QThread>
#include <QMutex>

#include "infra/asteriastate.h"
#include "infra/image.h"
#include "infra/ringbuffer.h"

/**
 * @brief The AcquisitionState enum enumerates the possible states of the
 * acquisition thread.
 */
enum AcquisitionState{DETECTING, RECORDING, IDLE};

/**
 * @brief The CalibrationState enum enumerates the possible states of the
 * calibration thread, which can be accumulating frames for processing with
 * the calibration routine, or can be idle.
 */
enum CalibrationState{CALIBRATING, NOT_CALIBRATING};

class AcquisitionThread : public QThread
{
    Q_OBJECT

public:
    AcquisitionThread(QObject *parent = 0, AsteriaState * state = 0);
    ~AcquisitionThread();

signals:
    void acquiredImage(std::shared_ptr<Image>);
    void acquiredClip(std::string utc);

public slots:

    /**
     * @brief launch Start the thread if it's not currently running
     */
    void launch();
    /**
     * @brief shutdown Shutdown the thread and release all resources
     */
    void shutdown();
    /**
     * @brief pause Pause image acquisition
     */
    void pause();
    /**
     * @brief resume Resume image acquisition from the paused state
     */
    void resume();


protected:
    void run() Q_DECL_OVERRIDE;

private:

    AsteriaState * state;

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
     * @brief calibrationFrames
     * Buffer to store the calibration footage
     */
    std::vector<std::shared_ptr<Image>> calibrationFrames;

    /**
     * @brief state
     * The current state of the acquisition thread, which determines what is done with newly
     * acquired frames.
     */
    AcquisitionState acqState;

    /**
     * @brief calState
     * The current state of the calibration procedure.
     */
    CalibrationState calState;

    /**
     * @brief calibration_intervals_frames
     * Number of frames between calibration intervals.
     */
    unsigned int calibration_intervals_frames;

    /**
     * @brief mutex used to control multithreaded use of instances of this class.
     */
    QMutex mutex;
};

#endif // ACQUISITIONTHREAD_H
