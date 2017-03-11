#ifndef METEORCAPTURESTATE_H
#define METEORCAPTURESTATE_H

#include <string>     // provides string

#include <linux/videodev2.h>

class MeteorCaptureState
{

public:

    // Cannot be loaded from config file: must be created programmatically,
    // either by user selection or automated selection of default camera.

    /**
     * \brief Path to the camera device file
     */
    std::string * cameraPath;

    /**
     * \brief Open file descriptor on the camera device
     */
    int * fd;

    /**
     * \brief Information about the video buffer(s) in use.
     */
    struct v4l2_buffer * bufferinfo;

    /**
     * \brief The pixel format in use.
     */
    struct v4l2_format * format;

    /**
     * \brief Information about requested & allocated buffers.
     */
    struct v4l2_requestbuffers * bufrequest;

    /**
     * \brief Array of pointers to the start of each image buffer in memory
     */
    char ** buffer_start;

    /**
     * \brief The image width
     */
    unsigned int width;

    /**
     * \brief The image height
     */
    unsigned int height;

    MeteorCaptureState() {
        bufferinfo = new v4l2_buffer();
        format = new v4l2_format();
        bufrequest = new v4l2_requestbuffers();
    }

    ~MeteorCaptureState() {
        delete bufferinfo;
        delete format;
        delete bufrequest;
        delete buffer_start;
    }
};

#endif // METEORCAPTURESTATE_H
