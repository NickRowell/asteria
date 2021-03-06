#ifndef ACQUISITIONTHREAD_H
#define ACQUISITIONTHREAD_H

#include "infra/asteriastate.h"
#include "infra/imageuc.h"
#include "infra/ringbuffer.h"
#include "infra/concurrentqueue.h"
#include "infra/acquisitionvideostats.h"

#include <linux/videodev2.h>
#include <vector>
#include <memory>               // shared_ptr
#include <string>

#include <QThread>
#include <QMutex>

class AcquisitionThread : public QThread
{
    Q_OBJECT

public:
    AcquisitionThread(QObject *parent = 0, AsteriaState * state = 0);
    ~AcquisitionThread();

    /**
     * @brief The AcquisitionState enum enumerates the possible states of the
     * acquisition thread.
     */
    enum AcquisitionState{PREVIEWING, PAUSED, DETECTING, RECORDING, CALIBRATING};
    static const std::string acquisitionStateNames[];

    /**
     * @brief The Action enum
     * Enumerates actions supplied by user or conditions within the algorithm that cause the
     * acqusition to transition between states.
     */
    enum Action{PREVIEW, PAUSE, DETECT};
    static const std::string actionNames[];

signals:
    void acquiredImage(std::shared_ptr<Imageuc>, bool renderOverlay, bool renderTopField, bool renderBottomField);
    void videoStats(const AcquisitionVideoStats &);
    void acquiredClip(std::string utc);
    void acquiredCalibration(std::string utc);
    void transitionedToState(AcquisitionThread::AcquisitionState);

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
     * @brief preview Preview images live; don't run detection.
     */
    void preview();
    /**
     * @brief pause Pause image acquisition.
     */
    void pause();
    /**
     * @brief detect Start auto-detection.
     */
    void detect();

    /**
     * @brief Used to toggle the rendering of the overlay image, in GUI mode. Sometimes it's
     * useful to turn it off.
     * @param checkBoxState
     *  The state of the checkbox presented to the user.
     */
    void toggleOverlay(int checkBoxState);

    /**
     * @brief Replaces the CalibrationInventory currently in use with the given one.
     * @param cal
     *  A shared_ptr to the new CalibrationInventory
     */
    void updateCalibration(std::shared_ptr<CalibrationInventory> cal);

protected:
    void run() Q_DECL_OVERRIDE;

private:

    /**
     * @brief The main state object.
     */
    AsteriaState * state;

    /**
     * \brief Information about the video buffer(s) in use.
     * See https://www.linuxtv.org/downloads/legacy/video4linux/API/V4L2_API/spec/ch03s05.html
     */
    struct v4l2_buffer * bufferinfo;

    /**
     * \brief The pixel format in use.
     */
    struct v4l2_format * format;

    /**
     * \brief Information about requested & allocated buffers.
     */
    struct v4l2_requestbuffers * bufrequest;

    /**
     * @brief abort
     * Flag used to abort the acquisition thread and shutdown.
     */
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
    RingBuffer<std::shared_ptr<Imageuc>> detectionHeadBuffer;

    /**
     * @brief eventFrames
     * Buffer to store the detection and 'detection tail' footage
     */
    std::vector<std::shared_ptr<Imageuc>> eventFrames;

    /**
     * @brief calibrationFrames
     * Buffer to store the calibration footage
     */
    std::vector<std::shared_ptr<Imageuc>> calibrationFrames;

    /**
     * @brief state
     * The current state of the acquisition thread, which determines what is done with newly
     * acquired frames.
     */
    AcquisitionState acqState;

    /**
     * @brief actions
     * Queue of actions to perform. Implemented as a queue because more than one action may be
     * provided within the space of one frame, and the thread should handle each in turn to avoid
     * concurrency problems.
     */
    ConcurrentQueue<Action> actions;

    /**
     * @brief calibration_intervals_frames
     * Number of frames between calibration intervals.
     */
    unsigned int calibration_intervals_frames;

    /**
     * @brief max_clip_length_frames
     * Maximum number of frames for a clip.
     */
    unsigned int max_clip_length_frames;

    /**
     * @bIndicates whether we're displaying the overlay imagerief showOverlayImage
     *
     */
    bool showOverlayImage;

    /**
     * @brief mutex used to control multithreaded use of instances of this class.
     */
    QMutex mutex;

    /**
     * @brief transitionToState
     * Function used to perform state transitions internally, so we can log whenever they happen
     */
    void transitionToState(AcquisitionThread::AcquisitionState);
};

#endif // ACQUISITIONTHREAD_H
