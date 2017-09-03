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
#include <cmath>                // round(...)

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

    format = new v4l2_format();
    memset(format, 0, sizeof(*format));
    format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format->fmt.pix.pixelformat = state->selectedFormat;
    format->fmt.pix.width = state->width;
    format->fmt.pix.height = state->height;

    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_S_FMT, format) < 0) {
        perror("VIDIOC_S_FMT");
        ::close(*(this->state->fd));
        exit(1);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //      Determine interlaced/progressive scan mode       //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    switch(format->fmt.pix.field) {
    case V4L2_FIELD_ANY:
    case V4L2_FIELD_TOP:
    case V4L2_FIELD_BOTTOM:
    case V4L2_FIELD_SEQ_TB:
    case V4L2_FIELD_SEQ_BT:
    case V4L2_FIELD_ALTERNATE:
        // Not supported!
        fprintf(stderr, "Image field format %s not supported!\n", V4L2Util::getV4l2FieldNameFromIndex(format->fmt.pix.field).c_str());
        ::close(*(this->state->fd));
        exit(1);
        break;
    case V4L2_FIELD_NONE:
    case V4L2_FIELD_INTERLACED:
    case V4L2_FIELD_INTERLACED_TB:
    case V4L2_FIELD_INTERLACED_BT:
        // Supported!
        fprintf(stderr, "Image field format %s is supported\n", V4L2Util::getV4l2FieldNameFromIndex(format->fmt.pix.field).c_str());
        break;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //        Determine nominal period between frames        //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // There are several uses for this:
    //  - detection of dropped frames, from the time interval between consecutive frames
    //  - setting the maximum number of frames in a clip from the maximum allowed time
    //  - setting the number of frames between calibration runs

    struct v4l2_streamparm parm;
    memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_G_PARM, &parm) < 0) {
        perror("VIDIOC_G_PARM");
        ::close(*(this->state->fd));
        exit(1);
    }
    // This struct holds the stream capture parameters
    v4l2_captureparm cparm = parm.parm.capture;
    unsigned int numerator = cparm.timeperframe.numerator;
    unsigned int denominator = cparm.timeperframe.denominator;

    double framePeriodSecs = (double)numerator / (double)denominator;
    this->state->nominalFramePeriodUs = framePeriodSecs * 1000000;

    // Assume shutter speed (exposure time) is the same as the frame period
    this->state->nominalExposureTimeUs = this->state->nominalFramePeriodUs;

    fprintf(stderr, "Time per frame = %d / %d  (%f) [seconds] (%d [microseconds]) \n", numerator, denominator, framePeriodSecs, this->state->nominalFramePeriodUs);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    // Determine any relevant image parameters from V4L2 API //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // TODO: do I want to use any of these?
    V4L2Util::printUserControls(*(this->state->fd));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //  Determine number of frames between calibration runs  //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    calibration_intervals_frames = (1.0 / framePeriodSecs) * 60 * state->calibration_interval;

    fprintf(stderr, "Interval between calibration runs = %d [frames]\n", calibration_intervals_frames);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //    Determine maximum number of frames for any clip    //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    max_clip_length_frames = (1.0 / framePeriodSecs) * 60 * state->clip_max_length;

    fprintf(stderr, "Maximum length of a clip = %d [frames]\n", max_clip_length_frames);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //     Inform device about buffers & streaming mode      //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Inform device about buffers to use
    bufrequest = new v4l2_requestbuffers();
    memset(bufrequest, 0, sizeof(*bufrequest));
    bufrequest->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest->memory = V4L2_MEMORY_MMAP;
    bufrequest->count = 32;

    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_REQBUFS, bufrequest) < 0){
        perror("VIDIOC_REQBUFS");
        ::close(*(this->state->fd));
        exit(1);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //  Determine memory requirements and allocate buffers   //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Here, the device informs us how much memory is required for the buffers
    // given the image format, frame dimensions and number of buffers.

    bufferinfo = new v4l2_buffer();
    memset(bufferinfo, 0, sizeof(*bufferinfo));

    // Array of pointers to the start of each buffer in memory
    buffer_start = new unsigned char*[bufrequest->count];

    for(unsigned int b = 0; b < bufrequest->count; b++) {

        bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo->memory = V4L2_MEMORY_MMAP;
        bufferinfo->index = b;

        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_QUERYBUF, bufferinfo) < 0){
            perror("VIDIOC_QUERYBUF");
            exit(1);
        }

        // bufferinfo.length: number of bytes of memory required for the buffer
        // bufferinfo.m.offset: offset from the start of the device memory for this buffer
        buffer_start[b] = (unsigned char *)mmap(NULL, bufferinfo->length, PROT_READ | PROT_WRITE, MAP_SHARED, *(this->state->fd), bufferinfo->m.offset);

        if(buffer_start[b] == MAP_FAILED){
            perror("mmap");
            exit(1);
        }

        memset(buffer_start[b], 0, bufferinfo->length);
    }

}

AcquisitionThread::~AcquisitionThread()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();

    fprintf(stderr, "Deactivating streaming...\n");
    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(bufferinfo->type)) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(1);
    }

    fprintf(stderr, "Deallocating image buffers...\n");
    for(unsigned int b = 0; b < bufrequest->count; b++) {
        if(munmap(buffer_start[b], bufferinfo->length) < 0) {
            perror("munmap");
        }
    }
    delete buffer_start;

    fprintf(stderr, "Deleting V4L2 structs...\n");
    delete bufferinfo;
    delete format;
    delete bufrequest;

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

void AcquisitionThread::toggleOverlay(int checkBoxState) {
    switch(checkBoxState) {
    case Qt::Checked:
        showOverlayImage = true;
        break;
    case Qt::Unchecked:
        showOverlayImage = false;
        break;
    }
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
    // Counter for dropped frames
    unsigned int droppedFramesCounter = 0;
    // Records capture time of the previous frame, for detecting frame drops
    long long lastFrameCaptureTime = 0ll;

    unsigned long i = 0;
    forever {

        if(abort) {
            return;
        }

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
                    for(unsigned long k = 0; k<bufrequest->count; k++) {
                        bufferinfo->index = k;
                        bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        bufferinfo->memory = V4L2_MEMORY_MMAP;
                        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_QBUF, bufferinfo) < 0){
                            perror("VIDIOC_QBUF");
                            exit(1);
                        }
                    }
                    fprintf(stderr, "Activating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMON, &(bufferinfo->type)) < 0){
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
                    // Abort recording; don't save the partial results
                    eventFrames.clear();
                    nFramesSinceLastTrigger = 0;
                    transitionToState(PREVIEWING);
                    break;
                case CALIBRATING:
                    // Abort calibration; don't save the partial results
                    calibrationFrames.clear();
                    transitionToState(PREVIEWING);
                    break;
                }
                break;
            case PAUSE:
                fprintf(stderr, "Performing action PAUSE\n");
                switch(acqState) {
                case PREVIEWING:
                    // Turn off streaming; transition to PAUSED
                    fprintf(stderr, "Deactivating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(bufferinfo->type)) < 0){
                        perror("VIDIOC_STREAMOFF");
                        exit(1);
                    }
                    i=0;
                    frameCaptureTimes.clear();
                    detectionHeadBuffer.clear();
                    transitionToState(PAUSED);
                    break;
                case PAUSED:
                    // No change
                    break;
                case DETECTING:
                    // Turn off streaming; transition to PAUSED
                    fprintf(stderr, "Deactivating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(bufferinfo->type)) < 0){
                        perror("VIDIOC_STREAMOFF");
                        exit(1);
                    }
                    i=0;
                    frameCaptureTimes.clear();
                    detectionHeadBuffer.clear();
                    transitionToState(PAUSED);
                    break;
                case RECORDING:
                    // Turn off streaming; transition to PAUSED
                    fprintf(stderr, "Deactivating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(bufferinfo->type)) < 0){
                        perror("VIDIOC_STREAMOFF");
                        exit(1);
                    }
                    i=0;
                    frameCaptureTimes.clear();
                    detectionHeadBuffer.clear();
                    // Abort recording; don't save the partial results
                    eventFrames.clear();
                    nFramesSinceLastTrigger = 0;
                    transitionToState(PAUSED);
                    break;
                case CALIBRATING:
                    // Turn off streaming; transition to PAUSED
                    fprintf(stderr, "Deactivating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(bufferinfo->type)) < 0){
                        perror("VIDIOC_STREAMOFF");
                        exit(1);
                    }
                    i=0;
                    frameCaptureTimes.clear();
                    detectionHeadBuffer.clear();
                    // Abort calibration; don't save the partial results
                    calibrationFrames.clear();
                    transitionToState(PAUSED);
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
                    for(unsigned long k = 0; k<bufrequest->count; k++) {
                        bufferinfo->index = k;
                        bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        bufferinfo->memory = V4L2_MEMORY_MMAP;
                        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_QBUF, bufferinfo) < 0){
                            perror("VIDIOC_QBUF");
                            exit(1);
                        }
                    }
                    fprintf(stderr, "Activating streaming...\n");
                    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMON, &(bufferinfo->type)) < 0){
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
            QThread::usleep(state->nominalFramePeriodUs);
            continue;
        }

        // Index into circular buffer
        unsigned int j = (i++) % bufrequest->count;

        bufferinfo->index = j;
        bufferinfo->memory = V4L2_MEMORY_MMAP;

        // Wait for this buffer to be dequeued then retrieve the image
        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_DQBUF, bufferinfo) < 0){
            perror("VIDIOC_DQBUF");
            exit(1);
        }

        // The image is ready to be read; it is stored in the buffer with index j,
        // which is mapped into application address space at buffer_start[j]

        // System clock time (since startup/hibernation) of time first byte of data was captured [microseconds]
        long long temp_us = 1000000LL * bufferinfo->timestamp.tv_sec + (long long) round(bufferinfo->timestamp.tv_usec);
        // Translate to microseconds since 1970-01-01T00:00:00Z
        long long epochTimeStamp_us = temp_us +  state->epochTimeDiffUs;

        string utc = TimeUtil::convertToUtcString(epochTimeStamp_us);

        std::shared_ptr<Image> image = make_shared<Image>(state->width, state->height);
        image->epochTimeUs = epochTimeStamp_us;
        image->field = format->fmt.pix.field;

        switch(format->fmt.pix.pixelformat) {
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
                JpgUtil::convertJpeg((unsigned char *)buffer_start[j], bufferinfo->bytesused, image->rawImage);
                break;
            }
            case V4L2_PIX_FMT_YUYV: {
                // Convert the YUYV (luminance + chrominance) image to greyscale
                JpgUtil::convertYuyv422((unsigned char *)buffer_start[j], bufferinfo->bytesused, image->rawImage);
                break;
            }
        }

        // TODO: if the frame number i is less than the number of frames to flush, skip the rest of the
        // loop.

        // Monitor FPS and dropped FPS, after the first 10 frames
        if(i > 2) {
            frameCaptureTimes.push(epochTimeStamp_us);
        }
        if(i > 10) {

            long long observedFramePeriodUs = epochTimeStamp_us - lastFrameCaptureTime;
            // Number of frames periods since the last frame was captured; detects dropped frames
            unsigned int frames = std::round((float)observedFramePeriodUs / (float)state->nominalFramePeriodUs);
            // Difference of more than 1 between consecutive frames indicates that frame(s) have been dropped
            droppedFramesCounter += (frames - 1);
            // Compute FPS
            double timeDiffSec = (frameCaptureTimes.back() - frameCaptureTimes.front()) / 1000000.0;
            fps = (frameCaptureTimes.size()-1) / timeDiffSec;

            if(state->headless) {
                // Headless mode: print frame stats to console
                fprintf(stderr, "+++ FPS: %06f Dropped: %06d Total: %06lu +++\n", fps, droppedFramesCounter, i);
            }
        }
        lastFrameCaptureTime = epochTimeStamp_us;

        AcquisitionVideoStats stats(fps, droppedFramesCounter, i, utc);

        // Re-enqueue the buffer now we've extracted all the image data
        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_QBUF, bufferinfo) < 0){
            perror("VIDIOC_QBUF");
            exit(1);
        }

        // Retrieve the previous image...
        std::shared_ptr<Image> prev = detectionHeadBuffer.back();
        // ...then add the current image to the buffer.
        detectionHeadBuffer.push(image);

        if(acqState==PREVIEWING) {
            // PREVIEWING - don't proceed to event detection and calibration.
            emit acquiredImage(image, true, true, true);
            emit videoStats(stats);
            continue;
        }

        // Any other state - DETECTING, RECORDING, CALIBRATING - we now check for event
        // occurrence between the current frame and the previous one.
        bool event = false;

        if(prev) {

            // Events are detected by counting the number of pixels with significant
            // changes in brightness. If this is above a threshold then an event is detected.
            unsigned int nChangedPixels = 0;

            for(unsigned int p=0; p< state->width * state->height; p++) {

                unsigned char newPixel = image->rawImage[p];
                unsigned char oldPixel = prev->rawImage[p];

                if(abs(newPixel - oldPixel) > state->pixel_difference_threshold) {
                    nChangedPixels++;
                    if(newPixel - oldPixel > 0) {
                        image->changedPixelsPositive.push_back(p);
                    }
                    else {
                        image->changedPixelsNegative.push_back(p);
                    }
                }
            }

            if(nChangedPixels > state->n_changed_pixels_for_trigger) {
                event = true;
                if(acqState != RECORDING) {
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
            else if(nFramesSinceLastCalibration >= calibration_intervals_frames) {
                transitionToState(CALIBRATING);
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
                // Notify listeners when a new clip is available
                connect(worker, SIGNAL(finished(std::string)), this, SIGNAL(acquiredClip(std::string)));
                thread->start();

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
                // Abort calibration: the calibration algorithms assume the signal is stable, and are compromised
                // by the occurence of events in the scene.
                calibrationFrames.clear();
                // Transition to RECORDING to capture the event
                transitionToState(RECORDING);
                // Copy the detection head buffer contents to the event frames buffer
                std::vector<std::shared_ptr<Image>> detectionHeadFrames = detectionHeadBuffer.unroll();
                eventFrames.insert(eventFrames.end(), detectionHeadFrames.begin(), detectionHeadFrames.end());
            }
            else {
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
                    // Notify listeners when a new calibration is available
                    connect(worker, SIGNAL(finished(std::string)), this, SIGNAL(acquiredCalibration(std::string)));
                    thread->start();

                    // Clear the calibration buffer, reset the counter
                    calibrationFrames.clear();
                    nFramesSinceLastCalibration = 0;

                    // Back to DETECTING state
                    transitionToState(DETECTING);
                }
            }
        }

        if(!state->headless && showOverlayImage) {
            image->generateAnnotatedImage();
        }

        // Notify attached listeners that a new frame is available
        emit acquiredImage(image, showOverlayImage, true, true);
        emit videoStats(stats);
    }

}
