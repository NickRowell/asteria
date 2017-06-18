#include "replayvideothread.h"

ReplayVideoThread::ReplayVideoThread() : frames(0), idx(0), state(STOPPED), abort(false), fperiodms(40) {
    // Start the thread
    start(NormalPriority);
}

ReplayVideoThread::~ReplayVideoThread() {
    mutex.lock();
    abort = true;
    mutex.unlock();
    wait();
}

void ReplayVideoThread::loadClip(std::vector<std::shared_ptr<Image> > images) {

    // Stop any current replay
    stop();

    // Clear any existing frames
    frames.clear();
    // Push new frames
    frames.insert(frames.end(), images.begin(), images.end());
    // Reset counter
    idx = 0;
}

void ReplayVideoThread::play() {
    state = PLAYING;
}

void ReplayVideoThread::pause() {
    state = PAUSED;
}

void ReplayVideoThread::stop() {
    state = STOPPED;
}

void ReplayVideoThread::run() {

    forever {

        if(abort) {
            return;
        }

        // Take no action if we have no video
        if(!frames.empty()) {
            // Take action depending on the ReplayState
            switch(state) {
            case PLAYING:
                // Check we've not yet reached the end
                if(idx==frames.size()) {
                    // Reset to start; stop playing
                    idx=0;
                    state = STOPPED;
                }
                else {
                    emit queueNewFrame(frames[idx++]);
                }
                break;
            case STOPPED:
                idx=0;
                break;
            case PAUSED:
                break;
            }
        }

        // Delay for one frame period
        QThread::msleep(fperiodms);
    }
}
