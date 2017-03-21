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


    // TODO: much of this should be moved to different class

    // Should separate genuine parameters of the system etc from temporary
    // objects created during operation of the software.


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


    // Application parameters (read from command line, affect behaviour of application)
    // e.g. run headless
    //      config directory location
    /**
     * \brief Boolean flag to indicate if we're running without a GUI.
     */
    bool headless;

    // System parameters
    // Location of JPL ephemeris, Earth topology model, maps etc

    // Station parameters

    /**
     * \brief Station longitude [decimal degrees]
     */
    double longitude;

    /**
     * \brief Station latitude [decimal degrees]
     */
    double latitude;




    // Camera parameters

    // Detection parameters

    // Analysis parameters


















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
