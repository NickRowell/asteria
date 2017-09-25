#include "gui/videoplayerthread.h"
#include "util/timeutil.h"


VideoPlayerThread::VideoPlayerThread(const unsigned int &framePeriodUs) : frames(0), idx(0), topField(true), state(STOPPED), abort(false), framePeriodUs(framePeriodUs) {
    // Start the thread
    start(NormalPriority);
}

VideoPlayerThread::~VideoPlayerThread() {
    mutex.lock();
    abort = true;
    mutex.unlock();
    wait();
}

void VideoPlayerThread::loadClip(std::vector<std::shared_ptr<Image> > images, std::shared_ptr<Image> splash) {

    // Stop any current replay
    stop();

    // Clear any existing frames
    frames.clear();
    // Push new frames
    frames.insert(frames.end(), images.begin(), images.end());
    // Store splash image
    this->splash = splash;
    // Reset counter
    idx = 0;
    // Compute clip length
    long long clipLengthUs = frames.back()->epochTimeUs - frames.front()->epochTimeUs;
    clipLengthSecs = (double) clipLengthUs / 1000000.0;

    // Reset de-interlaced stepping
    deinterlacedStepping = false;
}

void VideoPlayerThread::toggleDiStepping(int checkBoxState) {
    switch(checkBoxState) {
    case Qt::Checked:
        deinterlacedStepping = true;
        break;
    case Qt::Unchecked:
        deinterlacedStepping = false;
        break;
    }
}

void VideoPlayerThread::toggleOverlay(int checkBoxState) {
    switch(checkBoxState) {
    case Qt::Checked:
        showOverlayImage = true;
        break;
    case Qt::Unchecked:
        showOverlayImage = false;
        break;
    }
    state = FQUEUED;
}

void VideoPlayerThread::play() {
    state = PLAYING;
}

void VideoPlayerThread::pause() {
    state = PAUSED;
}

void VideoPlayerThread::stop() {
    state = STOPPED;
}

void VideoPlayerThread::stepf() {
    state = STEPF;
}

void VideoPlayerThread::stepb() {
    state = STEPB;
}

void VideoPlayerThread::queueFrameIndex(int fIdx) {
    state = FQUEUED;
    idx = fIdx;
}

void VideoPlayerThread::processFrame(unsigned int fIdx, std::shared_ptr<Image> image, bool isTopField, bool isBottomField) {

    // Compute AnalysisVideoStats

    // Time of this frame in the clip
    long long framePositionUs = image->epochTimeUs - frames.front()->epochTimeUs;
    double framePositionSecs = (double) framePositionUs / 1000000.0;
    std::string utc = TimeUtil::epochToUtcString(image->epochTimeUs);

    AnalysisVideoStats stats(clipLengthSecs, frames.size(), framePositionSecs, fIdx, isTopField, isBottomField, utc);

    emit videoStats(stats);
    emit queueNewFrame(image, showOverlayImage, isTopField, isBottomField);
    emit queuedFrameIndex(fIdx);
}

void VideoPlayerThread::run() {

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
                if(idx==frames.size()-1) {
                    // Stop playing
                    state = STOPPED;
                }
                else {
                    idx++;
                    processFrame(idx, frames[idx], true, true);
                }
                break;
            case STOPPED:
                idx=0;
                processFrame(idx, splash, true, true);
                break;
            case PAUSED:
                break;
            case STEPB:
                // Step backwards to previous frame, or previous field if we're in de-interlaced stepping mode
                if(deinterlacedStepping && !topField) {
                    // Step back to the top field in the same frame
                    topField = true;
                    processFrame(idx, frames[idx], topField, !topField);
                }
                else if(idx > 0) {
                    --idx;
                    if(deinterlacedStepping) {
                        // Stepping backwards - start on bottom field of previous frame
                        topField = false;
                        processFrame(idx, frames[idx], topField, !topField);
                    }
                    else {
                        processFrame(idx, frames[idx], true, true);
                    }
                }
                // Return to PAUSED state to prevent recurrence of step
                state = PAUSED;
                break;
            case STEPF:
                // Step forwards to next frame, or next field if we're in de-interlaced stepping mode
                if(deinterlacedStepping && topField) {
                    // Step forwards to the bottom field in the same frame
                    topField = false;
                    processFrame(idx, frames[idx], topField, !topField);
                }
                else if(idx < (frames.size()-1)) {
                    ++idx;
                    if(deinterlacedStepping) {
                        // Stepping forwards - start on top field of next frame
                        topField = true;
                        processFrame(idx, frames[idx], topField, !topField);
                    }
                    else {
                        processFrame(idx, frames[idx], true, true);
                    }
                }
                // Return to PAUSED state to prevent recurrence of step
                state = PAUSED;
                break;
            case FQUEUED:
                processFrame(idx, frames[idx], true, true);
                break;
            }
        }

        // Delay for one frame period
        QThread::usleep(framePeriodUs);
    }
}
