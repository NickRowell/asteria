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

AcquisitionThread::AcquisitionThread(QObject *parent, MeteorCaptureState * state)
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
    state->bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    state->bufferinfo->memory = V4L2_MEMORY_MMAP;

    // Array of pointers to the start of each buffer in memory
    buffer_start = new unsigned char*[state->bufrequest->count];

    for(unsigned int b = 0; b < state->bufrequest->count; b++) {
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

    qInfo() << "Closing the camera...";
    ::close(*(this->state->fd));

    delete buffer_start;
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

    // TMP: counter for number of frames received from camera
    int frameCounter = 0;
    // Timestamp - difference used to determine average FPS
    long long startUs = TimeUtil::getUpTime();

    for(unsigned long i = 0; ; i++) {

        if(abort) {
            long long endUs = TimeUtil::getUpTime();
            long long dUs = endUs - startUs;
            float secs = ((float)dUs) / 1000000.0f;
            // Number of frames per second on average
            float fps = frameCounter / secs;
            qInfo() << "Seconds of up time = " << secs;
            qInfo() << "Average FPS = " << fps;
            return;
        }

        // Index into circular buffer
        unsigned int j = i % state->bufrequest->count;

        state->bufferinfo->index = j;

        // Wait for this buffer to be dequeued then retrieve the image
        if(ioctl(*(this->state->fd), VIDIOC_DQBUF, state->bufferinfo) < 0){
            perror("VIDIOC_DQBUF");
            exit(1);
        }

        frameCounter++;

        // The image is ready to be read; it is stored in the buffer with index j,
        // which is mapped into application address space at buffer_start[j]

        // Retrieve the timestamp and frame index for this image

        qInfo() << "Sequence  = " << state->bufferinfo->sequence;
//        qInfo() << "Timestamp = " << state->bufferinfo->timestamp.tv_sec << " [s] " << state->bufferinfo->timestamp.tv_usec << " [usec]";

#if 1

        // System clock time (since startup/hibernation) of time first byte of data was captured [microseconds]
        long long temp_us = 1000000LL * state->bufferinfo->timestamp.tv_sec + (long long) round(  state->bufferinfo->timestamp.tv_usec);
        // Translate to microseconds since 1970-01-01T00:00:00Z
        long long epochTimeStamp_us = temp_us +  state->epochTimeDiffUs;

        string utc = TimeUtil::convertToUtcString(epochTimeStamp_us);

        std::shared_ptr<Image> image = make_shared<Image>(state->width, state->height);

        image->epochTimeUs = epochTimeStamp_us;

        switch(state->format->fmt.pix.pixelformat) {
        case V4L2_PIX_FMT_GREY: {
            // Read the raw greyscale pixels to the image object
            unsigned char * pBuf = buffer_start[j];
            unsigned int nPix = state->width * state->height;
            for(unsigned int p=0; p<nPix; p++) {
                image->pixelData.push_back(*(pBuf++));
            }
            break;
        }
        case V4L2_PIX_FMT_MJPEG: {
            // Convert the JPEG image to greyscale
            JpgUtil::convertJpeg((unsigned char *)buffer_start[j], state->bufferinfo->bytesused, image->pixelData);
            break;
        }
        case V4L2_PIX_FMT_YUYV: {
            // Convert the YUYV (luminance + chrominance) image to greyscale
            JpgUtil::convertYuyv422((unsigned char *)buffer_start[j], state->bufferinfo->bytesused, image->pixelData);
            break;
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

                unsigned char newPixel = image->pixelData[p];
                unsigned char oldPixel = prev->pixelData[p];

                if(abs(newPixel - oldPixel) > state->pixel_difference_threshold) {
                    nChangedPixels++;

                    // Coordinates of changed pixel
                    // TODO: verify this - are the pixels packed by row?
                    unsigned int x = p % state->width;
                    unsigned int y = p / state->width;
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
                QThread* thread = new QThread;
                AnalysisWorker* worker = new AnalysisWorker(NULL, this->state, eventFrames);
                worker->moveToThread(thread);
                connect(thread, SIGNAL(started()), worker, SLOT(process()));
                connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
                connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
                connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
                thread->start();

                // Clear the event frame buffer
                eventFrames.clear();
            }
        }

        // Notify attached listeners that a new frame is available
        emit acquiredImage(image);
#endif

    }

}
