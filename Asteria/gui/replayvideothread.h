#ifndef REPLAYVIDEOTHREAD_H
#define REPLAYVIDEOTHREAD_H

#include "infra/image.h"

#include <memory>

#include <QThread>
#include <QMutex>

/**
 * Current state of the replay viewer
 * @brief The ReplayState enum
 */
enum ReplayState{PLAYING, PAUSED, STOPPED, STEPF, STEPB};



class ReplayVideoThread : public QThread {
    Q_OBJECT

public:
    ReplayVideoThread();
    ~ReplayVideoThread();

    // The clip we're playing
    std::vector<std::shared_ptr<Image>> frames;

    // The index of the frame to display next
    unsigned int idx;

    // Current state
    ReplayState state;

    // Flag used to close down the thread
    bool abort;

    // Delay between frames for 25 FPS or whatever
    unsigned long fperiodms;

    /**
     * @brief mutex used to control multithreaded use of instances of this class.
     */
    QMutex mutex;

protected:
    void run() Q_DECL_OVERRIDE;

public slots:
    void loadClip(std::vector<std::shared_ptr<Image>>);

    void play();
    void pause();
    void stop();
    void stepb();
    void stepf();

signals:
    void queueNewFrame(std::shared_ptr<Image> image);


};

#endif // REPLAYVIDEOTHREAD_H
