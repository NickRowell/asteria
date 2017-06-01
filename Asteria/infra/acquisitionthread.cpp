#include "infra/acquisitionthread.h"
#include "infra/analysisworker.h"
#include "util/jpgutil.h"
#include "util/timeutil.h"

#include <linux/videodev2.h>
#include <sys/ioctl.h>          // IOCTL etc
#include <sys/mman.h>           // mmap etc
#include <memory>               // shared_ptr
#include <sstream>              // ostringstream

#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <algorithm>            // std::find(...)

#include <QDebug>
#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

AcquisitionThread::AcquisitionThread(QObject *parent, AsteriaState * state)
    : QThread(parent), state(state), detectionHeadBuffer(state->detection_head), abort(false) {

    acqState = IDLE;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //      Set the image size & format for the camera       //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    state->format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    state->format->fmt.pix.pixelformat = state->selectedFormat;
    state->format->fmt.pix.width = state->width;
    state->format->fmt.pix.height = state->height;

    if(ioctl(*(this->state->fd), VIDIOC_S_FMT, state->format) < 0) {
        perror("VIDIOC_S_FMT");
        ::close(*(this->state->fd));
        exit(1);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //     Inform device about buffers & streaming mode      //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Inform device about buffers to use
    state->bufrequest->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    state->bufrequest->memory = V4L2_MEMORY_MMAP;
    state->bufrequest->count = 32;

    if(ioctl(*(this->state->fd), VIDIOC_REQBUFS, state->bufrequest) < 0){
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

}

AcquisitionThread::~AcquisitionThread()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();

    qInfo() << "Deactivating streaming...";
    if(ioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(state->bufferinfo->type)) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(1);
    }

    qInfo() << "Deallocating image buffers...";
    for(unsigned int b = 0; b < state->bufrequest->count; b++) {
        if(munmap(buffer_start[b], state->bufferinfo->length) < 0) {
            perror("munmap");
        }
    }
    delete buffer_start;

    qInfo() << "Closing the camera...";
    ::close(*(this->state->fd));
}

void AcquisitionThread::launch() {

    // Lock this object for the duration of this function
    QMutexLocker locker(&mutex);

    if (!isRunning()) {
        start(NormalPriority);
    }
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

        if(ioctl(*(this->state->fd), VIDIOC_QBUF, state->bufferinfo) < 0){
            perror("VIDIOC_QBUF");
            exit(1);
        }
    }

    if(ioctl(*(this->state->fd), VIDIOC_STREAMON, &(state->bufferinfo->type)) < 0){
        perror("VIDIOC_STREAMON");
        exit(1);
    }

    acqState = DETECTING;

    // Monitor the FPS using a ringbuffer to buffer the image capture times and get a moving average
    RingBuffer<long long> frameCaptureTimes(1000u);
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
        if(ioctl(*(this->state->fd), VIDIOC_DQBUF, state->bufferinfo) < 0){
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
            qInfo() << "FPS   = " << fps;
            qInfo() << "Dropped frames = " << droppedFramesCounter;
            qInfo() << "Total frames = " << totalFramesCounter;
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
        if(ioctl(*(this->state->fd), VIDIOC_QBUF, state->bufferinfo) < 0){
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
                qInfo() << "EVENT! " << utc.c_str();
            }
        }

        // We always accumulate the new images in the ring buffer regardless of whether
        // we're currently recording an event. This supports two events in rapid succession.
        detectionHeadBuffer.push(image);

        if(acqState == IDLE) {
            // Do nothing
        }
        else if(acqState == DETECTING) {
            if(event) {
                // We're detecting and an event occurred - transition to recording mode
                // and start accumulating images in the detection tail buffer
                qInfo() << "Transitioned to RECORDING";
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
                qInfo() << "Transitioned to DETECTING";
                qInfo() << "Got " << eventFrames.size() << " frames from last event";

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

        // Notify attached listeners that a new frame is available
        emit acquiredImage(image);

    }

}
