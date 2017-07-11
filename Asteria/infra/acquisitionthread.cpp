#include "infra/acquisitionthread.h"
#include "infra/analysisworker.h"
#include "infra/calibrationworker.h"
#include "util/jpgutil.h"
#include "util/timeutil.h"
#include "util/ioutil.h"
#include "util/v4l2util.h"

#include <linux/videodev2.h>
//#include <sys/ioctl.h>          // IOCTL etc
#include <sys/mman.h>           // mmap etc
#include <memory>               // shared_ptr
#include <sstream>              // ostringstream

#include <fstream>
#include <iostream>
#include <iomanip>              // setprecision(...)

#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <algorithm>            // std::find(...)

#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

const std::string AcquisitionThread::acquisitionStateNames[] = {"PREVIEWING", "PAUSED", "DETECTING", "RECORDING", "CALIBRATING"};
const std::string AcquisitionThread::actionNames[] = {"PREVIEW", "PAUSE", "DETECT"};

AcquisitionThread::AcquisitionThread(QObject *parent, AsteriaState * state)
    : QThread(parent), state(state), detectionHeadBuffer(state->detection_head), abort(false) {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //           Load the reference star catalogue           //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    std::vector<ReferenceStar> refStarCatalogue = ReferenceStar::loadCatalogue(state->refStarCataloguePath);

    fprintf(stderr, "Loaded %d ReferenceStars!\n", refStarCatalogue.size());

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //      Set the image size & format for the camera       //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    state->format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    state->format->fmt.pix.pixelformat = state->selectedFormat;
    state->format->fmt.pix.width = state->width;
    state->format->fmt.pix.height = state->height;

    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_S_FMT, state->format) < 0) {
        perror("VIDIOC_S_FMT");
        ::close(*(this->state->fd));
        exit(1);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //      Determine interlaced/progressive scan mode       //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    switch(state->format->fmt.pix.field) {
    case V4L2_FIELD_ANY:
    case V4L2_FIELD_TOP:
    case V4L2_FIELD_BOTTOM:
    case V4L2_FIELD_SEQ_TB:
    case V4L2_FIELD_SEQ_BT:
    case V4L2_FIELD_ALTERNATE:
        // Not supported!
        fprintf(stderr, "Image field format %s not supported!\n", V4L2Util::getV4l2FieldNameFromIndex(state->format->fmt.pix.field).c_str());
        ::close(*(this->state->fd));
        exit(1);
        break;
    case V4L2_FIELD_NONE:
    case V4L2_FIELD_INTERLACED:
    case V4L2_FIELD_INTERLACED_TB:
    case V4L2_FIELD_INTERLACED_BT:
        // Supported!
        fprintf(stderr, "Image field format %s is supported\n", V4L2Util::getV4l2FieldNameFromIndex(state->format->fmt.pix.field).c_str());
        break;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //  Determine exposure time & whether it's configurable  //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // TODO
    double expTimeSeconds = 0.04; // 25 FPS

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //  Determine number of frames between calibration runs  //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    calibration_intervals_frames = (1.0 / expTimeSeconds) * 60 * state->calibration_interval;

    fprintf(stderr, "Interval between calibration frames: %d\n", calibration_intervals_frames);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //     Inform device about buffers & streaming mode      //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Inform device about buffers to use
    state->bufrequest->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    state->bufrequest->memory = V4L2_MEMORY_MMAP;
    state->bufrequest->count = 32;

    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_REQBUFS, state->bufrequest) < 0){
        perror("VIDIOC_REQBUFS");
        ::close(*(this->state->fd));
        exit(1);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //    Determine maximum number of frames for any clip    //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    max_clip_length_frames = (1.0 / expTimeSeconds) * 60 * state->clip_max_length;

    fprintf(stderr, "Maximum length of a clip: %d frames\n", max_clip_length_frames);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //  Determine memory requirements and allocate buffers   //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Here, the device informs us how much memory is required for the buffers
    // given the image format, frame dimensions and number of buffers.


    // Array of pointers to the start of each buffer in memory
    buffer_start = new unsigned char*[state->bufrequest->count];

    for(unsigned int b = 0; b < state->bufrequest->count; b++) {

        state->bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        state->bufferinfo->memory = V4L2_MEMORY_MMAP;
        state->bufferinfo->index = b;

        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_QUERYBUF, state->bufferinfo) < 0){
            perror("VIDIOC_QUERYBUF");
            exit(1);
        }

        // bufferinfo.length: number of bytes of memory required for the buffer
        // bufferinfo.m.offset: offset from the start of the device memory for this buffer
        buffer_start[b] = (unsigned char *)mmap(NULL, state->bufferinfo->length, PROT_READ | PROT_WRITE, MAP_SHARED, *(this->state->fd), state->bufferinfo->m.offset);

        if(buffer_start[b] == MAP_FAILED){
            perror("mmap");
            exit(1);
        }

        memset(buffer_start[b], 0, state->bufferinfo->length);
    }

}

AcquisitionThread::~AcquisitionThread()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();

    fprintf(stderr, "Deactivating streaming...\n");
    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(state->bufferinfo->type)) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(1);
    }

    fprintf(stderr, "Deallocating image buffers...\n");
    for(unsigned int b = 0; b < state->bufrequest->count; b++) {
        if(munmap(buffer_start[b], state->bufferinfo->length) < 0) {
            perror("munmap");
        }
    }
    delete buffer_start;

    fprintf(stderr, "Closing the camera...\n");
    ::close(*(this->state->fd));
}

void AcquisitionThread::launch() {

    // Lock this object for the duration of this function
    QMutexLocker locker(&mutex);

    if (!isRunning()) {
        start(NormalPriority);
    }
}

void AcquisitionThread::shutdown() {

    // Lock this object for the duration of this function
    QMutexLocker locker(&mutex);

    fprintf(stderr, "Shutting down!\n");
    if (isRunning()) {
        abort = true;
    }
}

void AcquisitionThread::preview() {
    QMutexLocker locker(&mutex);
    actions.push(PREVIEW);
}

void AcquisitionThread::pause() {
    QMutexLocker locker(&mutex);
    actions.push(PAUSE);
}

void AcquisitionThread::detect() {
    QMutexLocker locker(&mutex);
    actions.push(DETECT);
}

void AcquisitionThread::transitionToState(AcquisitionThread::AcquisitionState newState) {
    acqState = newState;
    emit transitionedToState(acqState);
    fprintf(stderr, "Transitioned to %s\n", AcquisitionThread::acquisitionStateNames[acqState].c_str());
}

void AcquisitionThread::run() {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //                 Activate streaming                    //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Start in PAUSED state
    acqState = PAUSED;
    // ... queue up a DETECT action to initiate thread in DETECTING mode
    actions.push(DETECT);

    // The number of frames recorded since the last trigger. Usually, there will be
    // multiple triggers during a single event, so we reset this counter to zero on each trigger
    // and terminate the recording when it exceeds the detection tail length.
    unsigned int nFramesSinceLastTrigger = 0;

    // Counts the number of frames since we last calibrated
    unsigned int nFramesSinceLastCalibration = 0;

    // Monitor the FPS using a ringbuffer to buffer the image capture times and get a moving average
    RingBuffer<long long> frameCaptureTimes(100u);
    double fps = 0.0;
    // Monitor dropped FPS: more tricky as we don't know the capture times of the dropped frames
    unsigned int droppedFramesCounter = 0;
    unsigned int totalFramesCounter = 0;
    unsigned int lastFrameSequence = 0;

    unsigned long i = 0;
    forever {

        if(abort) {
            return;
        }

        // TODO:
        // Should plain old 'record' button be available? How would that interact with other controls?
        // What about CALIBRATING? Should this run in PREVIEW state?
        // What happens if we're recording a calibration stack when the user hits pause (?)
        // What happens if we're recording a clip when the user hits pause?

        // Check if there's an action to perform
        Action action;
        if(actions.pop(action)) {
            // action now contains the action to perform
            switch(action) {
            case PREVIEW:
                fprintf(stderr, "Performing action PREVIEW\n");
                switch(acqState) {
                case PREVIEWING:
                    // No change
                    break;
                case PAUSED:
                    // Turn on streaming; transition to PREVIEWING
                    fprintf(stderr, "Adding buffers to incoming queue...\n");
                    for(unsigned long k = 0; k<state->bufrequest->count; k++) {
                        state->bufferinfo->index = k;
                        state->bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        state->bufferinfo->memory = V4L2_MEMORY_MMAP;
                        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_QBUF, state->bufferinfo) < 0){
                            perror("VIDIOC_QBUF");
                            exit(1);
                        }
                    }
                    fprintf(stderr, "Activating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMON, &(state->bufferinfo->type)) < 0){
                        perror("VIDIOC_STREAMON");
                        exit(1);
                    }
                    transitionToState(PREVIEWING);
                    break;
                case DETECTING:
                    // Already streaming; transition to PREVIEWING
                    transitionToState(PREVIEWING);
                    break;
                case RECORDING:
                    // TODO:  WHAT TO DO IF WE'RE RECORDING A CLIP WHEN USER HITS PREVIEW?
                    break;
                case CALIBRATING:
                    // TODO: WHAT TO DO IF WE'RE CALIBRATING WHEN USER HITS PREVIEW?
                    break;
                }

                break;
            case PAUSE:
                fprintf(stderr, "Performing action PAUSE\n");
                switch(acqState) {
                case PREVIEWING:
                    // Turn off streaming; transition to PAUSED
                    fprintf(stderr, "Deactivating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(state->bufferinfo->type)) < 0){
                        perror("VIDIOC_STREAMOFF");
                        exit(1);
                    }
                    i=0;
                    detectionHeadBuffer.clear();
                    transitionToState(PAUSED);
                    break;
                case PAUSED:
                    // No change
                    break;
                case DETECTING:
                    // Turn off streaming; transition to PAUSED
                    fprintf(stderr, "Deactivating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(state->bufferinfo->type)) < 0){
                        perror("VIDIOC_STREAMOFF");
                        exit(1);
                    }
                    i=0;
                    detectionHeadBuffer.clear();
                    transitionToState(PAUSED);
                    break;
                case RECORDING:
                    // TODO: WHAT TO DO IF WE'RE RECORDING A CLIP WHEN USER HITS PAUSE?
                    break;
                case CALIBRATING:
                    // TODO: WHAT TO DO IF WE'RE CALIBRATING WHEN USER HITS PAUSE?
                    break;
                }
                break;
            case DETECT:
                fprintf(stderr, "Performing action DETECT\n");
                switch(acqState) {
                case PREVIEWING:
                    // Already streaming; transition to DETECTING
                    transitionToState(DETECTING);
                    break;
                case PAUSED:
                    // Turn on streaming; transition to DETECTING
                    fprintf(stderr, "Adding buffers to incoming queue...\n");
                    for(unsigned long k = 0; k<state->bufrequest->count; k++) {
                        state->bufferinfo->index = k;
                        state->bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        state->bufferinfo->memory = V4L2_MEMORY_MMAP;
                        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_QBUF, state->bufferinfo) < 0){
                            perror("VIDIOC_QBUF");
                            exit(1);
                        }
                    }
                    fprintf(stderr, "Activating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMON, &(state->bufferinfo->type)) < 0){
                        perror("VIDIOC_STREAMON");
                        exit(1);
                    }
                    transitionToState(DETECTING);
                    break;
                case DETECTING:
                    // No change
                    break;
                case RECORDING:
                    // No change
                    break;
                case CALIBRATING:
                    // No change
                    break;
                }
                break;
            }
        }

        // Now proceed according to the current AcquisitionState
        if(acqState==PAUSED) {
            // TODO: set the delay correctly
            QThread::msleep(40);
            continue;
        }

        // Index into circular buffer
        unsigned int j = (i++) % state->bufrequest->count;

        state->bufferinfo->index = j;
        state->bufferinfo->memory = V4L2_MEMORY_MMAP;

        // Wait for this buffer to be dequeued then retrieve the image
        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_DQBUF, state->bufferinfo) < 0){
            perror("VIDIOC_DQBUF");
            exit(1);
        }

        // The image is ready to be read; it is stored in the buffer with index j,
        // which is mapped into application address space at buffer_start[j]

        // System clock time (since startup/hibernation) of time first byte of data was captured [microseconds]
        long long temp_us = 1000000LL * state->bufferinfo->timestamp.tv_sec + (long long) round(  state->bufferinfo->timestamp.tv_usec);
        // Translate to microseconds since 1970-01-01T00:00:00Z
        long long epochTimeStamp_us = temp_us +  state->epochTimeDiffUs;

        // Monitor FPS and dropped FPS, skipping the first 2 frames as these seem to often have incorrect sequence numbers and/or time stamps
        if(i > 1) {
            // Difference of more than 1 between consecutive frames indicates that frame(s) have been dropped
            droppedFramesCounter += state->bufferinfo->sequence - (lastFrameSequence + 1);
            totalFramesCounter += state->bufferinfo->sequence - lastFrameSequence;
            frameCaptureTimes.push(epochTimeStamp_us);
            double timeDiffSec = (frameCaptureTimes.back() - frameCaptureTimes.front()) / 1000000.0;
            fps = (frameCaptureTimes.size()-1) / timeDiffSec;

            if(state->headless) {
                // Headless mode: print frame stats to console
                fprintf(stderr, "+++ FPS: %06f Dropped: %06d Total: %06d +++\n", fps, droppedFramesCounter, totalFramesCounter);
            }
        }
        lastFrameSequence = state->bufferinfo->sequence;

        VideoStats stats(fps, droppedFramesCounter, totalFramesCounter);

        string utc = TimeUtil::convertToUtcString(epochTimeStamp_us);

        std::shared_ptr<Image> image = make_shared<Image>(state->width, state->height);
        image->epochTimeUs = epochTimeStamp_us;
        image->field = state->format->fmt.pix.field;

        switch(state->format->fmt.pix.pixelformat) {
            case V4L2_PIX_FMT_GREY: {
                // Read the raw greyscale pixels to the image object
                unsigned char * pBuf = buffer_start[j];
                unsigned int nPix = state->width * state->height;
                for(unsigned int p=0; p<nPix; p++) {
                    image->rawImage[p] = *(pBuf++);
                }
                break;
            }
            case V4L2_PIX_FMT_MJPEG: {
                // Convert the JPEG image to greyscale
                JpgUtil::convertJpeg((unsigned char *)buffer_start[j], state->bufferinfo->bytesused, image->rawImage);
                break;
            }
            case V4L2_PIX_FMT_YUYV: {
                // Convert the YUYV (luminance + chrominance) image to greyscale
                JpgUtil::convertYuyv422((unsigned char *)buffer_start[j], state->bufferinfo->bytesused, image->rawImage);
                break;
            }
        }

        // Write the grey pixels to the annotated image
        if(!state->headless) {
            for(unsigned int p = 0; p < state->width * state->height; p++) {
                unsigned char pixel = image->rawImage[p];
                unsigned int pix32bit = (pixel << 24) + (pixel << 16) + (pixel << 8) + (255 << 0);
                image->annotatedImage[p] = (pix32bit);
            }
        }

        // Re-enqueue the buffer now we've extracted all the image data
        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_QBUF, state->bufferinfo) < 0){
            perror("VIDIOC_QBUF");
            exit(1);
        }

        // Retrieve the previous image...
        std::shared_ptr<Image> prev = detectionHeadBuffer.back();
        // ...then add the current image to the buffer.
        detectionHeadBuffer.push(image);

        if(acqState==PREVIEWING) {
            // PREVIEWING - don't proceed to event detection and calibration.
            emit acquiredImage(image);
            emit videoStats(stats);
            continue;
        }

        // Any other state - DETECTING, RECORDING, CALIBRATING - we now check for event
        // occurrence between the current frame and the previous one.
        bool event = false;

        if(prev) {

            // Events are detected by counting the number of pixels with significant
            // changes in brightness. If this is above a threshold then an event is detected.
            int nChangedPixels = 0;

            for(unsigned int p=0; p< state->width * state->height; p++) {

                unsigned char newPixel = image->rawImage[p];
                unsigned char oldPixel = prev->rawImage[p];

                if(abs(newPixel - oldPixel) > state->pixel_difference_threshold) {
                    nChangedPixels++;
                    // Indicate the changed pixel in the annotated image
                    if(!state->headless) {
                        image->annotatedImage[p] = 0x0000FFFF;
                    }
                }
            }

            if(nChangedPixels > state->n_changed_pixels_for_trigger) {
                event = true;
                if(state->headless && acqState != RECORDING) {
                    // TODO: Instead, log whenever state changes
                    // Only print one event notification for each recording
                    fprintf(stderr, "EVENT! %s\n", utc.c_str());
                }
            }
        }

        nFramesSinceLastCalibration++;

        // Process the acquisition
        if(acqState == DETECTING) {
            // Transition to RECORDING if we've detected an event
            if(event) {
                transitionToState(RECORDING);
                // Copy the detection head buffer contents to the event frames buffer
                std::vector<std::shared_ptr<Image>> detectionHeadFrames = detectionHeadBuffer.unroll();
                eventFrames.insert(eventFrames.end(), detectionHeadFrames.begin(), detectionHeadFrames.end());
            }

            // Transition to CALIBRATING if counter has reached (or passed) limit
            if(nFramesSinceLastCalibration >= calibration_intervals_frames) {
                transitionToState(CALIBRATING);
                nFramesSinceLastCalibration = 0;
            }
        }
        else if(acqState == RECORDING) {

            // Add the image to the event frames buffer
            eventFrames.push_back(image);

            // Increment the counter
            nFramesSinceLastTrigger++;

            if(event) {
                // We're recording and an event occurred - reset the counter
                nFramesSinceLastTrigger = 0;
            }

            // Stop recording if we hit the upper limit on clip length, or when enough frames have passed
            // since the last detected event.
            if(eventFrames.size() >= max_clip_length_frames || nFramesSinceLastTrigger > state->detection_tail) {
                // Create an AnalysisWorker to analyse the clip in a dedicated thread
                QThread* thread = new QThread;
                AnalysisWorker* worker = new AnalysisWorker(NULL, this->state, eventFrames);
                worker->moveToThread(thread);
                connect(thread, SIGNAL(started()), worker, SLOT(process()));
                connect(worker, SIGNAL(finished(std::string)), thread, SLOT(quit()));
                connect(worker, SIGNAL(finished(std::string)), worker, SLOT(deleteLater()));
                connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
                thread->start();

                // Notify listeners when a new clip is available
                connect(worker, SIGNAL(finished(std::string)), this, SIGNAL(acquiredClip(std::string)));

                // Clear the event frame buffer
                eventFrames.clear();

                // Reset counter
                nFramesSinceLastTrigger = 0;

                // Back to DETECTING state
                transitionToState(DETECTING);
            }
        }
        else if(acqState == CALIBRATING) {

            if(event) {
                // TODO: what happens if an event is detected while recording calibration frames?
                // Should we terminate the calibration on the grounds that the images are compromised?
            }

            // Add the frame to the calibration set
            calibrationFrames.push_back(image);

            // Determine if we've recorded all the calibration frames we need
            if(calibrationFrames.size() >= state->calibration_stack) {
                // Got enough frames: run calibration algorithm
                QThread* thread = new QThread;
                CalibrationWorker* worker = new CalibrationWorker(NULL, this->state, calibrationFrames);
                worker->moveToThread(thread);
                connect(thread, SIGNAL(started()), worker, SLOT(process()));
                connect(worker, SIGNAL(finished(std::string)), thread, SLOT(quit()));
                connect(worker, SIGNAL(finished(std::string)), worker, SLOT(deleteLater()));
                connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
                thread->start();

                // Clear the calibration buffer
                calibrationFrames.clear();

                // Back to DETECTING state
                transitionToState(DETECTING);
            }

        }

        // Notify attached listeners that a new frame is available
        emit acquiredImage(image);
        emit videoStats(stats);
    }

}
