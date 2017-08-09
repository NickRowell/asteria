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
     * Current state of the replay viewer
     * @brief The ReplayState enum
     */
    enum ReplayState{PLAYING, PAUSED, STOPPED, STEPF, STEPB, FQUEUED};

    // The clip we're playing
    std::vector<std::shared_ptr<Image>> frames;

    // Peak hold image, to display before the clip starts and whenever it stops
    std::shared_ptr<Image> peakHold;

    // The total length of the clip [secs]
    double clipLengthSecs;

    // The index of the frame currently being displayed
    unsigned int idx;

    // Indicates whether we're in de-interlaced stepping mode
    bool deinterlacedStepping;

    // Indicates whether we're displaying the overlay image
    bool showOverlayImage;

    // Indicates the field (top=true/bottom=false) currently being displayed. This is only relevant when in
    // de-interlaced stepping mode
    bool topField = true;

    // Current state
    ReplayState state;

    // Flag used to close down the thread
    bool abort;

    // Delay between frames [microseconds]
    unsigned int framePeriodUs;

    /**
     * @brief mutex used to control multithreaded use of instances of this class.
     */
    QMutex mutex;

protected:
    void run() Q_DECL_OVERRIDE;

    void processFrame(unsigned int fIdx, std::shared_ptr<Image> image, bool isTopField, bool isBottomField);

public slots:
    void loadClip(std::vector<std::shared_ptr<Image>>, std::shared_ptr<Image>);
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
