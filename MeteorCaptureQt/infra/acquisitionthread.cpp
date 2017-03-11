#include "acquisitionthread.h"
#include "util/jpgutil.h"

#include <linux/videodev2.h>
#include <sys/ioctl.h>          // IOCTL etc
#include <sys/mman.h>           // mmap etc

#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

#include <QDebug>
#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

AcquisitionThread::AcquisitionThread(QObject *parent, MeteorCaptureState * state)
    : QThread(parent)
{
    this->state = state;
    abort = false;
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
}

void AcquisitionThread::launch() {

    // Lock this object for the duration of this function
    QMutexLocker locker(&mutex);

    if (!isRunning()) {
        start(LowPriority);
    }
}


void AcquisitionThread::run() {

    mutex.lock();
    mutex.unlock();

//    qInfo() << "Launching camera " << QString::fromStdString(*(state->cameraPath));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //        Set the image format for the camera            //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    struct v4l2_format * format = state->format;
    format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // Suitable for Watec camera:
//    format->fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
    // Suitable for webcam:
    format->fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format->fmt.pix.width = 640;
    format->fmt.pix.height = 480;

    if(ioctl(*(this->state->fd), VIDIOC_S_FMT, format) < 0) {
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
    struct v4l2_requestbuffers * bufrequest = state->bufrequest;
    bufrequest->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest->memory = V4L2_MEMORY_MMAP;
    bufrequest->count = 32;

    if(ioctl(*(this->state->fd), VIDIOC_REQBUFS, bufrequest) < 0){
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
    struct v4l2_buffer * bufferinfo = state->bufferinfo;

    memset(bufferinfo, 0, sizeof(*bufferinfo));

    bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo->memory = V4L2_MEMORY_MMAP;

//	bufferinfo.flags = bufferinfo.flags | V4L2_BUF_FLAG_TIMECODE;

    // Array of pointers to the start of each buffer in memory
    state->buffer_start = new char*[bufrequest->count];

    for(unsigned int b = 0; b < bufrequest->count; b++) {
        bufferinfo->index = b;

        if(ioctl(*(this->state->fd), VIDIOC_QUERYBUF, bufferinfo) < 0){
            perror("VIDIOC_QUERYBUF");
            exit(1);
        }

//        fprintf(stdout, "bufferinfo.length = %d\nbufferinfo.m.offset = %d\n", bufferinfo.length, bufferinfo.m.offset);

        // bufferinfo.length: number of bytes of memory required for the buffer
        // bufferinfo.m.offset: offset from the start of the device memory for this buffer
        state->buffer_start[b] = (char *)mmap(NULL, bufferinfo->length, PROT_READ | PROT_WRITE, MAP_SHARED, *(this->state->fd), bufferinfo->m.offset);

        if(state->buffer_start[b] == MAP_FAILED){
            perror("mmap");
            exit(1);
        }

        memset(state->buffer_start[b], 0, bufferinfo->length);

//        qInfo() <<  "Buffer length [bytes]:" << bufferinfo->length;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //                 Activate streaming                    //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    if(ioctl(*(this->state->fd), VIDIOC_STREAMON, &(bufferinfo->type)) < 0){
        perror("VIDIOC_STREAMON");
        exit(1);
    }

    //////////////////////////////////// TO OPERATE METHOD:

    // Write frames into circular buffer in device memory
    for(unsigned long i = 0; ; i++) {

        if(abort) {
            return;
        }

        // Index into circular buffer
        unsigned int j = i % bufrequest->count;

//        qInfo() << "buffer index = " << j;

        bufferinfo->index = j;

        // Put the buffer in the incoming queue.
        if(ioctl(*(this->state->fd), VIDIOC_QBUF, bufferinfo) < 0){
            perror("VIDIOC_QBUF");
            exit(1);
        }

        // The buffer's waiting in the outgoing queue.
        bufferinfo->flags = V4L2_BUF_FLAG_TIMECODE;
        if(ioctl(*(this->state->fd), VIDIOC_DQBUF, bufferinfo) < 0){
            perror("VIDIOC_QBUF");
            exit(1);
        }

        // The image is ready to be read; it is stored in the buffer with index j,
        // which is mapped into application address space at buffer_start[j]

        // Retrieve the timestamp and frame index for this image
        bufferinfo->timestamp;  // A struct timeval
        bufferinfo->timecode;   // A struct v4l2_timecode
        bufferinfo->sequence;   // __u32 recording the sequence count of this frame

        time_t timer;
        tm * gmt = gmtime (&timer);

//        qInfo() << "Sequence  = " << bufferinfo->sequence;
//        qInfo() << "Timestamp = " << bufferinfo->timestamp.tv_sec << " [s] " << bufferinfo->timestamp.tv_usec << " [usec]";

//		cout << "Timecode: " << endl;
//		cout << "  frames  = " << bufferinfo.timecode.frames << endl;
//		cout << "  hours   = " << bufferinfo.timecode.hours << endl;
//		cout << "  minutes = " << bufferinfo.timecode.minutes << endl;
//		cout << "  seconds = " << bufferinfo.timecode.seconds << endl;

        // Microseconds of the current time of day
//        long temp_ms = 1000000 * bufferinfo.timestamp.tv_sec + (long) round(  bufferinfo.timestamp.tv_usec);
//        // Microseconds after 1970-01-01T00:00:00Z
//        long epochTimeStamp_us = temp_ms +  epochTimeDiffUs;

//        long US_PER_DAY = 24l * 60l * 60l * 1000000l;
//        long days = epochTimeStamp_us / US_PER_DAY;
//        long years = days / 365;
//        long daysPastTheYear = days % years;

//        printf( "The frame's timestamp in epoch ms is: %ld \n", epochTimeStamp_us);
//        printf( "Years/Days =  %ld / %ld \n", years, daysPastTheYear);



        // Write the image data out to a JPEG file
        int imgFileHandle;
        char filename [100];

        switch(format->fmt.pix.pixelformat) {
        case V4L2_PIX_FMT_GREY:
            sprintf(filename, "/home/nick/Temp/myimage_%lu_%u.pgm", i, j);
            break;
        case V4L2_PIX_FMT_MJPEG:
            sprintf(filename, "/home/nick/Temp/myimage_%lu_%u.jpeg", i, j);
            break;
        }


        if((imgFileHandle = open(filename, O_WRONLY | O_CREAT, 0660)) < 0){
            perror("open");
            exit(1);
        }

        switch(format->fmt.pix.pixelformat) {
        case V4L2_PIX_FMT_GREY: {

            // Pointer to start of buffer data
            char * bufStart = (char *)state->buffer_start[j];

            // Notify attached listeners that a new frame is available
            emit acquiredImage(bufStart);


//            repaint();

//            std::ofstream out(filename);
//            // Raw PGMs:
//            out << "P5\n" << "640" << " 480" << " 255\n";
//            for(unsigned int k=0; k<480; k++) {
//                for(unsigned int l=0; l<640; l++) {
//                    unsigned int offset = k*640 + l;
//                    // Pointer to the pixel data
//                    char * pPix = (char *)(bufStart+offset);
//                    // Cast to a char
//                    char pix = *pPix;
//                    out << pix;
//                }
//            }
//            out.close();
            break;
        }
        case V4L2_PIX_FMT_MJPEG: {

            // Convert the JPEG image to greyscale

            char * decodedImage = new char[640*480];

            JpgUtil::convertJpeg((unsigned char *)state->buffer_start[j], bufferinfo->length, decodedImage);

            emit acquiredImage(decodedImage);

//            write(imgFileHandle, state->buffer_start[j], bufferinfo->length);
            break;
        }
        }

////        ::close(imgFileHandle);
    }

}

