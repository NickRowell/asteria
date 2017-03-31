#include "acquisitionthread.h"
#include "util/jpgutil.h"

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
    : QThread(parent)
{
    this->state = state;
    abort = false;

    bufferinfo = new v4l2_buffer();
    memset(bufferinfo, 0, sizeof(*bufferinfo));

    format = new v4l2_format();
    memset(format, 0, sizeof(*format));

    bufrequest = new v4l2_requestbuffers();
    memset(bufrequest, 0, sizeof(*bufrequest));

    // Initialise the camera

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //    Determine available pixel formats and pick one     //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Pixel formats supported by the software, in order of preference
    // V4L2_PIX_FMT_GREY - Watec camera
    // V4L2_PIX_FMT_MJPEG - Many (all?) webcams
//    unsigned int preferredFormats[] = {V4L2_PIX_FMT_GREY, V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG};
    unsigned int preferredFormats[] = {V4L2_PIX_FMT_GREY, V4L2_PIX_FMT_MJPEG};
    unsigned int preferredFormatsN = 2;

    // Pixel formats provided by the camera
    std::vector< unsigned int > supportedFormats;

    // Chosen format
    unsigned int selectedFormat;

    struct v4l2_fmtdesc vid_fmtdesc;
    memset(&vid_fmtdesc, 0, sizeof(vid_fmtdesc));
    vid_fmtdesc.index = 0;
    // Only interested in pixel formats for video capture
    vid_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    const char *buf_types[] = {"VIDEO_CAPTURE","VIDEO_OUTPUT", "VIDEO_OVERLAY"};
    const char *flags[] = {"uncompressed", "compressed"};

    // Send the VIDIOC_ENUM_FM ioctl and print the results
    while( ioctl(*(this->state->fd), VIDIOC_ENUM_FMT, &vid_fmtdesc ) == 0 )
    {
        supportedFormats.push_back(vid_fmtdesc.pixelformat);

        // We got a video format/codec back
        qInfo() << QString("VIDIOC_ENUM_FMT(%1, %2)").arg(QString::number(vid_fmtdesc.index), buf_types[vid_fmtdesc.type-1]);
        qInfo() << QString("  index       : %1").arg(QString::number(vid_fmtdesc.index));
        qInfo() << QString("  type        : %1").arg(buf_types[vid_fmtdesc.type-1]);
        qInfo() << QString("  flags       : %1").arg(flags[vid_fmtdesc.flags]);
        QString qDesc;
        qDesc.sprintf("  description : %s", vid_fmtdesc.description);
        qInfo() << qDesc;
        QString fmt;
        fmt.sprintf("%c%c%c%c",vid_fmtdesc.pixelformat & 0xFF, (vid_fmtdesc.pixelformat >> 8) & 0xFF,
                    (vid_fmtdesc.pixelformat >> 16) & 0xFF, (vid_fmtdesc.pixelformat >> 24) & 0xFF);
        qInfo() << QString("%1").arg(fmt);

        // Increment the format descriptor index
        vid_fmtdesc.index++;
    }

    // TODO: handle the case where none of the provided formats are supported
    // From the provided formats, pick the one that is preferred
    for(unsigned int f=0; f < preferredFormatsN; f++) {

        unsigned int preferredFormat = preferredFormats[f];

        // Is the corresponding preferred format provided by the camera?
        if(std::find(supportedFormats.begin(), supportedFormats.end(), preferredFormat) != supportedFormats.end()) {
            selectedFormat = preferredFormat;
            QString fmt;
            fmt.sprintf("%c%c%c%c", selectedFormat & 0xFF, (selectedFormat >> 8) & 0xFF,
                        (selectedFormat >> 16) & 0xFF, (selectedFormat >> 24) & 0xFF);
            qInfo() << QString("Selected format: %1").arg(fmt);
            break;
        }
        else {
            // Preferred format is not supported
        }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //   Determine the available image sizes for the chosen  //
    //   pixel format and pick one                           //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    struct v4l2_frmsizeenum framesize;
    memset(&framesize, 0, sizeof(framesize));
    framesize.pixel_format = selectedFormat;
    framesize.index = 0;

    while( ioctl(*(this->state->fd), VIDIOC_ENUM_FRAMESIZES, &framesize ) == 0 )
    {
        // TODO: what are the implications of the different types?
        switch(framesize.type) {
        case V4L2_FRMSIZE_TYPE_DISCRETE: {

            unsigned int width  = framesize.discrete.width;
            unsigned int height = framesize.discrete.height;
            qInfo() << "V4L2_FRMSIZE_TYPE_DISCRETE: Width x Height = " << width << " x " << height;
            break;
        }
        case V4L2_FRMSIZE_TYPE_STEPWISE: {
            qInfo() << "V4L2_FRMSIZE_TYPE_STEPWISE not supported!";
            break;
        }

        }

        framesize.index++;
    }

    state->width = 1280;
    state->height = 720;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //        Set the image format for the camera            //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format->fmt.pix.pixelformat = selectedFormat;
    format->fmt.pix.width = state->width;
    format->fmt.pix.height = state->height;

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

    memset(bufferinfo, 0, sizeof(*bufferinfo));

    bufferinfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo->memory = V4L2_MEMORY_MMAP;

    // Array of pointers to the start of each buffer in memory
    buffer_start = new char*[bufrequest->count];

    for(unsigned int b = 0; b < bufrequest->count; b++) {
        bufferinfo->index = b;

        if(ioctl(*(this->state->fd), VIDIOC_QUERYBUF, bufferinfo) < 0){
            perror("VIDIOC_QUERYBUF");
            exit(1);
        }

        // bufferinfo.length: number of bytes of memory required for the buffer
        // bufferinfo.m.offset: offset from the start of the device memory for this buffer
        buffer_start[b] = (char *)mmap(NULL, bufferinfo->length, PROT_READ | PROT_WRITE, MAP_SHARED, *(this->state->fd), bufferinfo->m.offset);

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

    qInfo() << "Deactivating streaming...";
    if(ioctl(*(this->state->fd), VIDIOC_STREAMOFF, &(bufferinfo->type)) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(1);
    }

    qInfo() << "Closing the camera...";
    ::close(*(this->state->fd));

    delete bufferinfo;
    delete format;
    delete bufrequest;
    delete buffer_start;
}

void AcquisitionThread::launch() {

    // Lock this object for the duration of this function
    QMutexLocker locker(&mutex);

    if (!isRunning()) {
        start(LowPriority);
    }
}


void AcquisitionThread::run() {

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

        qInfo() << "Sequence  = " << bufferinfo->sequence;
        qInfo() << "Timestamp = " << bufferinfo->timestamp.tv_sec << " [s] " << bufferinfo->timestamp.tv_usec << " [usec]";

        // Current system clock time (since startup/hibernation) in microseconds
        long temp_us = 1000000 * bufferinfo->timestamp.tv_sec + (long) round(  bufferinfo->timestamp.tv_usec);
        // Microseconds after 1970-01-01T00:00:00Z
        long epochTimeStamp_us = temp_us +  state->epochTimeDiffUs;

        // Split into whole seconds and remainder microseconds
        long epochTimeStamp_s = epochTimeStamp_us / 1000000;
        long epochTimeStamp_us_remainder = epochTimeStamp_us % 1000000;

        // Convert the seconds part to time_t
        time_t tt = static_cast<time_t>(epochTimeStamp_s);

        // Use standard library function(s) to convert this to human readable date/time
        struct tm * ptm = gmtime ( &tt );

        // seconds after the minute	[0-61]*
        // * tm_sec is generally 0-59. The extra range is to accommodate for leap seconds in certain systems.
        int tm_sec = ptm->tm_sec;
        // minutes after the hour	0-59
        int tm_min = ptm->tm_min;
        // hours since midnight	[0-23]
        int tm_hour = ptm->tm_hour;
        // day of the month	[1-31]
        int tm_mday = ptm->tm_mday;
        // months since January	[0-11]
        int tm_mon = ptm->tm_mon;
        // years since 1900; convert to years since AD 0
        int tm_year = ptm->tm_year + 1900;

        // Construct date string
        std::ostringstream strs;
        strs << tm_year << "/" << tm_mon << "/" << tm_mday << "-" << tm_hour << ":" << tm_min << ":" << tm_sec << "." << epochTimeStamp_us_remainder;
        qInfo() << strs.str().c_str();

        auto image = make_shared<Image>(state->width, state->height);

        image->epochTimeUs = epochTimeStamp_us;

        switch(format->fmt.pix.pixelformat) {
        case V4L2_PIX_FMT_GREY: {
            // Read the raw greyscale pixels to the image object
            char * pBuf = buffer_start[j];
            unsigned int nPix = state->width * state->height;
            for(unsigned int p=0; p<nPix; p++) {
                image->pixelData.push_back(*(pBuf++));
            }
            break;
        }
        case V4L2_PIX_FMT_MJPEG: {
            // Convert the JPEG image to greyscale
            JpgUtil::convertJpeg((unsigned char *)buffer_start[j], bufferinfo->bytesused, image->pixelData);
            break;
        }
        case V4L2_PIX_FMT_YUYV: {
            // Convert the YUYV (luminance + chrominance) image to greyscale
            JpgUtil::convertYuyv422((unsigned char *)buffer_start[j], bufferinfo->bytesused, image->pixelData);
            break;
        }

        }

        // Notify attached listeners that a new frame is available
        emit acquiredImage(image);





        // Saving images to file:

        // Write the image data out to a JPEG file
//        int imgFileHandle;
//        char filename [100];

//        switch(format->fmt.pix.pixelformat) {
//        case V4L2_PIX_FMT_GREY:
//            sprintf(filename, "/home/nick/Temp/myimage_%lu_%u.pgm", i, j);
//            break;
//        case V4L2_PIX_FMT_MJPEG:
//            sprintf(filename, "/home/nick/Temp/myimage_%lu_%u.jpeg", i, j);
//            break;
//        }


//        if((imgFileHandle = open(filename, O_WRONLY | O_CREAT, 0660)) < 0){
//            perror("open");
//            exit(1);
//        }

        // PGM (grey image)
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


        // JPEG (write the raw buffer data to file rather than the converted greyscale image)
        //            write(imgFileHandle, state->buffer_start[j], bufferinfo->length);

//        ::close(imgFileHandle);

    }

}

