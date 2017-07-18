#include "gui/replayvideothread.h"

ReplayVideoThread::ReplayVideoThread(const unsigned int &framePeriodUs) : frames(0), idx(0), state(STOPPED), abort(false), framePeriodUs(framePeriodUs) {
    // Start the thread
    start(NormalPriority);
}

ReplayVideoThread::~ReplayVideoThread() {
    mutex.lock();
    abort = true;
    mutex.unlock();
    wait();
}

void ReplayVideoThread::loadClip(std::vector<std::shared_ptr<Image> > images, std::shared_ptr<Image> peakHold) {

    // Stop any current replay
    stop();

    // Clear any existing frames
    frames.clear();
    // Push new frames
    frames.insert(frames.end(), images.begin(), images.end());
    // Store peakHold
    this->peakHold = peakHold;
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

void ReplayVideoThread::stepf() {
    state = STEPF;
}

void ReplayVideoThread::stepb() {
    state = STEPB;
}

void ReplayVideoThread::queueFrameIndex(int fIdx) {
    state = FQUEUED;
    idx = fIdx;
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
                    emit queueNewFrame(peakHold);
                    emit queuedFrameIndex(idx);
                }
                else {
                    emit queueNewFrame(frames[idx]);
                    emit queuedFrameIndex(idx);
                    idx++;
                }
                break;
            case STOPPED:
                idx=0;
                emit queueNewFrame(peakHold);
                emit queuedFrameIndex(idx);
                break;
            case PAUSED:
                break;
            case STEPB:
                // Check we've not yet reached the start
                if(idx > 0) {
                    --idx;
                    emit queueNewFrame(frames[idx]);
                    emit queuedFrameIndex(idx);
                }
                // Return to PAUSED state to prevent recurrence of step
                state = PAUSED;
                break;
            case STEPF:
                // Check we've not yet reached the end
                if(idx < (frames.size()-1)) {
                    ++idx;
                    emit queueNewFrame(frames[idx]);
                    emit queuedFrameIndex(idx);
                }
                // Return to PAUSED state to prevent recurrence of step
                state = PAUSED;
                break;
            case FQUEUED:
                emit queueNewFrame(frames[idx]);
                emit queuedFrameIndex(idx);
                break;
            }
        }

        // Delay for one frame period
        QThread::usleep(framePeriodUs);
    }
}
