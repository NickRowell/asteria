#ifndef REPLAYVIDEOTHREAD_H
#define REPLAYVIDEOTHREAD_H

#include "infra/image.h"
#include "infra/analysisvideostats.h"

#include <memory>

#include <QThread>
#include <QMutex>

class ReplayVideoThread : public QThread {

    Q_OBJECT

public:

    ReplayVideoThread(const unsigned int &framePeriodUs);
    ~ReplayVideoThread();

    /**
     * @brief Enumerates the possible states of the replay viewer.
     */
    enum ReplayState{PLAYING, PAUSED, STOPPED, STEPF, STEPB, FQUEUED};

    /**
     * @brief The clip we're playing, stored as a sorted vector of shared_ptrs to Images.
     */
    std::vector<std::shared_ptr<Image>> frames;

    /**
     * @brief Image to display before the clip starts and whenever it stops.
     */
    std::shared_ptr<Image> splash;

    /**
     * @brief The total length of the clip [secs]
     */
    double clipLengthSecs;

    /**
     * @brief The index of the frame currently being displayed
     */
    unsigned int idx;

    /**
     * @brief Indicates whether we're in de-interlaced stepping mode
     */
    bool deinterlacedStepping;

    /**
     * @brief Indicates whether we're displaying the overlay image
     */
    bool showOverlayImage;

    /**
     * @brief Indicates the field (top=true/bottom=false) currently being displayed, when in de-interlaced stepping mode.
     */
    bool topField;

    /**
     * @brief Current state of the replay
     */
    ReplayState state;

    /**
     * @brief Flag used to close down the thread
     */
    bool abort;

    /**
     * @brief Delay between frames [microseconds]
     */
    unsigned int framePeriodUs;

    /**
     * @brief Mutex used to control multithreaded use of instances of this class.
     */
    QMutex mutex;

protected:
    void run() Q_DECL_OVERRIDE;

    void processFrame(unsigned int fIdx, std::shared_ptr<Image> image, bool isTopField, bool isBottomField);

public slots:
    /**
     * @brief Load the video clip and prepare for playback
     * @param images
     *      The individual frames of the video clip, in ascending time order.
     * @param splash
     *      The splash image, i.e. the image to display when the clip is stopped.
     */
    void loadClip(std::vector<std::shared_ptr<Image>> images, std::shared_ptr<Image> splash);
    void toggleDiStepping(int checkBoxState);
    void toggleOverlay(int checkBoxState);

    void play();
    void pause();
    void stop();
    void stepb();
    void stepf();
    void queueFrameIndex(int fIdx);

signals:
    void videoStats(const AnalysisVideoStats &);
    void queueNewFrame(std::shared_ptr<Image> image, bool, bool, bool);
    void queuedFrameIndex(int fIdx);
};

#endif // REPLAYVIDEOTHREAD_H
