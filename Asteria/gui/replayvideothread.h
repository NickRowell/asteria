#ifndef REPLAYVIDEOTHREAD_H
#define REPLAYVIDEOTHREAD_H

#include "infra/image.h"

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

    // The index of the frame to display next
    unsigned int idx;

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

public slots:
    void loadClip(std::vector<std::shared_ptr<Image>>, std::shared_ptr<Image>);

    void play();
    void pause();
    void stop();
    void stepb();
    void stepf();
    void queueFrameIndex(int fIdx);

signals:
    void queueNewFrame(std::shared_ptr<Image> image);
    void queuedFrameIndex(int fIdx);
};

#endif // REPLAYVIDEOTHREAD_H
