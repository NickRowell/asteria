#include "infra/acquisitionthread.h"
#include "infra/analysisworker.h"
#include "infra/calibrationworker.h"
#include "util/jpgutil.h"
#include "util/timeutil.h"
#include "util/ioutil.h"

#include <linux/videodev2.h>
#include <sys/ioctl.h>          // IOCTL etc
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

AcquisitionThread::AcquisitionThread(QObject *parent, AsteriaState * state)
    : QThread(parent), state(state), detectionHeadBuffer(state->detection_head), abort(false) {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //           Load the reference star catalogue           //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    std::vector<ReferenceStar> refStarCatalogue = ReferenceStar::loadCatalogue(state->refStarCataloguePath);

    fprintf(stderr, "Loaded %d ReferenceStars!\n", refStarCatalogue.size());

    // Other initialisation to do:
    // 1) Load ephemeris file?
    // 2) Load existing calibration data?
    //     - should really port this out to an initialisation function that both the headless and
    //       GUI mode can use to load these to the state object.

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

    // TODO!
    if(state->format->fmt.pix.field == V4L2_FIELD_ANY) {
        // This is used to request any one of the V4L2_FIELD_NONE, V4L2_FIELD_TOP, V4L2_FIELD_BOTTOM, or V4L2_FIELD_INTERLACED
        // and wouldn't be returned by the query.
        fprintf(stderr, "V4L2_FIELD_ANY\n");
    }
    if(state->format->fmt.pix.field == V4L2_FIELD_NONE) {
        fprintf(stderr, "V4L2_FIELD_NONE (progressive)\n");
    }
    if(state->format->fmt.pix.field == V4L2_FIELD_TOP) {
        fprintf(stderr, "V4L2_FIELD_TOP (top field only)\n");
    }
    if(state->format->fmt.pix.field == V4L2_FIELD_BOTTOM) {
        fprintf(stderr, "V4L2_FIELD_BOTTOM (bottom field only)\n");
    }
    if(state->format->fmt.pix.field == V4L2_FIELD_INTERLACED) {
        fprintf(stderr, "V4L2_FIELD_INTERLACED (top and bottom field interleaved)\n");
    }
    if(state->format->fmt.pix.field == V4L2_FIELD_SEQ_TB) {
        fprintf(stderr, "V4L2_FIELD_SEQ_TB (contains both top & bottom fields in that order)\n");
    }
    if(state->format->fmt.pix.field == V4L2_FIELD_SEQ_BT) {
        fprintf(stderr, "V4L2_FIELD_SEQ_BT (contains both bottom & top fields in that order)\n");
    }
    if(state->format->fmt.pix.field == V4L2_FIELD_ALTERNATE) {

        // The two fields of a frame are passed in separate buffers, in temporal order,
        // i. e. the older one first. To indicate the field parity (whether the current
        // field is a top or bottom field) the driver or application, depending on data
        // direction, must set struct v4l2_buffer field to V4L2_FIELD_TOP or V4L2_FIELD_BOTTOM.
        // Any two successive fields pair to build a frame. If fields are successive, without
        // any dropped fields between them (fields can drop individually), can be determined
        // from the struct v4l2_buffer sequence field. Image sizes refer to the frame, not
        // fields. This format cannot be selected when using the read/write I/O method.
        fprintf(stderr, "V4L2_FIELD_ALTERNATE\n");
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //  Determine exposure time & whether it's configurable  //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // TODO
    double expTime = 0.04; // 25 FPS


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //  Determine number of frames between calibration runs  //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    calibration_intervals_frames = (1.0 / expTime) * 60 * state->calibration_interval;

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

        if(ioctl(*(this->state->fd), VIDIOC_QUERYBUF, state->bufferinfo) < 0){
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

    acqState = IDLE;
    calState = NOT_CALIBRATING;
}

AcquisitionThread::~AcquisitionThread()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();

    fprintf(stderr, "Deactivating streaming...\n");
    if(ioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(state->bufferinfo->type)) < 0){
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

void AcquisitionThread::pause() {
    QMutexLocker locker(&mutex);
}

void AcquisitionThread::resume() {
    QMutexLocker locker(&mutex);
}






void AcquisitionThread::run() {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //                 Activate streaming                    //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Add all buffers to the incoming queue
    for(unsigned long i = 0; i<state->bufrequest->count; i++) {
        state->bufferinfo->index = i;
        state->bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        state->bufferinfo->memory = V4L2_MEMORY_MMAP;

        if(IoUtil::xioctl(*(this->state->fd), VIDIOC_QBUF, state->bufferinfo) < 0){
            perror("VIDIOC_QBUF");
            exit(1);
        }
    }

    if(IoUtil::xioctl(*(this->state->fd), VIDIOC_STREAMON, &(state->bufferinfo->type)) < 0){
        perror("VIDIOC_STREAMON");
        exit(1);
    }

    acqState = DETECTING;

    // The number of frames recorded since the last trigger. Usually, there will be
    // multiple triggers during a single event, so we reset this counter to zero on each trigger
    // and terminate the recording when it exceeds the detection tail length.
    unsigned int nFramesSinceLastTrigger = 0;

    // Counts the number of frames since we last
    unsigned int nFramesSinceLastCalibration = 0;

    // Monitor the FPS using a ringbuffer to buffer the image capture times and get a moving average
    RingBuffer<long long> frameCaptureTimes(100u);
    double fps = 0.0;
    // Monitor dropped FPS: more tricky as we don't know the capture times of the dropped frames
    unsigned int droppedFramesCounter = 0;
    unsigned int totalFramesCounter = 0;
    unsigned int lastFrameSequence = 0;

    for(unsigned long i = 0; ; i++) {

        if(abort) {
            return;
        }

        // Index into circular buffer
        unsigned int j = i % state->bufrequest->count;

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
                fprintf(stderr, "+++ FPS: %06d Dropped: %06d Total: %06d +++\n", fps, droppedFramesCounter, totalFramesCounter);
//                std::cout << "+++ FPS: " << std::setw(6) << std::setprecision(5) << left << fps
//                          << " Dropped: " << std::setw(6) << droppedFramesCounter
//                          << " Total: " << std::setw(6) << totalFramesCounter
//                          << " +++" << '\r' << std::flush;
            }
        }
        lastFrameSequence = state->bufferinfo->sequence;

        string utc = TimeUtil::convertToUtcString(epochTimeStamp_us);

        std::shared_ptr<Image> image = make_shared<Image>(state->width, state->height);

        image->epochTimeUs = epochTimeStamp_us;
        image->fps = fps;
        image->droppedFrames = droppedFramesCounter;
        image->totalFrames = totalFramesCounter;


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
            unsigned int nPix = state->width * state->height;
            for(unsigned int p=0; p<nPix; p++) {
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

        // Retrieve the previous image
        std::shared_ptr<Image> prev = detectionHeadBuffer.back();

        // Determine if an event has occurred between current frame and previous
        bool event = false;

        if(prev) {

            // Got a previous image

            // Parameters for detection:
            //  - maximum length of a single recording
            //  - hot pixel map
            //  - star field / real-time mask
            //  - ...?

            int nChangedPixels = 0;

            unsigned int nPix = state->width * state->height;
            for(unsigned int p=0; p<nPix; p++) {

                unsigned char newPixel = image->rawImage[p];
                unsigned char oldPixel = prev->rawImage[p];

                if(abs(newPixel - oldPixel) > state->pixel_difference_threshold) {
                    nChangedPixels++;

                    // Coordinates of changed pixel
                    // TODO: verify this - are the pixels packed by row?
                    unsigned int x = p % state->width;
                    unsigned int y = p / state->width;

                    // Indicate the changed pixel in the annotated image
                    if(!state->headless) {
                        image->annotatedImage[p] = 0x0000FFFF;
                    }
                }
            }

            if(nChangedPixels > state->n_changed_pixels_for_trigger) {
                event = true;
                if(state->headless && acqState != RECORDING) {
                    // Only print one event notification for each recording
                    fprintf(stderr, "EVENT! %s\n", utc.c_str());
//                    std::cout << '\n' << "EVENT! " << utc.c_str() << '\n' << std::flush;
                }
            }
        }

        // We always accumulate the new images in the ring buffer regardless of whether
        // we're currently recording an event. This supports two events in rapid succession.
        detectionHeadBuffer.push(image);

        // Process the acquisition
        if(acqState == IDLE) {
            // Do nothing
        }
        else if(acqState == DETECTING) {
            if(event) {
                // We're detecting and an event occurred - transition to recording mode
                // and start accumulating images in the detection tail buffer
                acqState = RECORDING;
                // Copy the detection head buffer contents to the event frames buffer
                std::vector<std::shared_ptr<Image>> detectionHeadFrames = detectionHeadBuffer.unroll();
                eventFrames.insert(eventFrames.end(), detectionHeadFrames.begin(), detectionHeadFrames.end());
            }
        }
        else if(acqState == RECORDING) {

            // Add the image to the event frames buffer
            eventFrames.push_back(image);

            if(event) {
                // We're recording and an event occurred - reset the counter
                nFramesSinceLastTrigger = 0;
            }
            else {
                // Event did not occur - increment the counter
                nFramesSinceLastTrigger++;
            }

            // Check if enough frames have passed since last trigger to stop the recording
            if(nFramesSinceLastTrigger > state->detection_tail) {
                // Stop the recording; send the images to an analysis thread instance;
                // reset the buffers and counters.
                acqState = DETECTING;
                nFramesSinceLastTrigger = 0;
//                qInfo() << "Got " << eventFrames.size() << " frames from last event";

                // TODO: store running analysis threads in a threadpool so can limit their number
                // OR: use one single analysis thread, and queue up analysis jobs for serial processing
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
            }
        }

        if(calState == CALIBRATING) {

            // Determine if we've recorded all the calibration frames we need
            if(calibrationFrames.size() > state->calibration_stack) {
                // Spawn a new CalibrationThread
                fprintf(stderr, "Got %d frames for calibration\n", calibrationFrames.size());

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

                // Back to NOT_CALIBRATING mode
                calState = NOT_CALIBRATING;
            }
            else {
                // Add the frame to the calibration set
                calibrationFrames.push_back(image);
            }

        }
        else if (calState = NOT_CALIBRATING) {
            // Not yet time to perform calibration increment frame counter
            nFramesSinceLastCalibration++;

            // Determine if the calibration should be started on the next frame
            if(nFramesSinceLastCalibration >= calibration_intervals_frames) {
                calState = CALIBRATING;
                nFramesSinceLastCalibration = 0;
            }
        }


        // Notify attached listeners that a new frame is available
        emit acquiredImage(image);

    }

}
