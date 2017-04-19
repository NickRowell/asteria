#ifndef METEORCAPTURESTATE_H
#define METEORCAPTURESTATE_H

#include <linux/videodev2.h>
#include <string>               // provides string
#include <cstring>              // memset

using namespace std;


class MeteorCaptureState
{

public:

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                    Application parameters                    //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Application parameters (read from command line, affect behaviour of application)
    // e.g. run headless
    //      config directory location
    /**
     * \brief Boolean flag to indicate if we're running without a GUI.
     */
    int headless = 0;

    // State associated with the operation and handling of the camera

    // Pixel formats supported by the software, in order of preference
    // V4L2_PIX_FMT_GREY - Watec camera
    // V4L2_PIX_FMT_MJPEG - Many (all?) webcams
    //    unsigned int preferredFormats[] = {V4L2_PIX_FMT_GREY, V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG};
    const unsigned int preferredFormats[2] = {V4L2_PIX_FMT_GREY, V4L2_PIX_FMT_MJPEG};
    const unsigned int preferredFormatsN = 2;

    // Chosen pixel format
    unsigned int selectedFormat;

    /**
     * \brief Information about the video buffer(s) in use.
     * See https://www.linuxtv.org/downloads/legacy/video4linux/API/V4L2_API/spec/ch03s05.html
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
     * \brief Path to the camera device file
     */
    std::string * cameraPath;

    /**
     * \brief Open file descriptor on the camera device
     */
    int * fd;


    // Cannot be loaded from config file: must be created programmatically,
    // either by user selection or automated selection of default camera.

    /**
     * @brief epochTimeDiffUs
     * Time difference between system clock time (since startup/hibernation) and the
     * current epoch time. Used to convert image time codes to UTC.
     */
    long long epochTimeDiffUs;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                      Camera parameters                       //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * \brief The image width
     */
    unsigned int width;

    /**
     * \brief The image height
     */
    unsigned int height;

    /**
     * \brief Camera azimuthal angle [decimal degrees]
     */
    double azimuth;

    /**
     * \brief Camera elevation angle [decimal degrees]
     */
    double elevation;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                      System parameters                       //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief configDirPath
     */
    string configDirPath;

    /**
     * @brief videoDirPath
     */
    string videoDirPath;

    /**
     * @brief refStarCataloguePath
     */
    string refStarCataloguePath;

    /**
     * @brief jplEphemerisPath
     */
    string jplEphemerisPath;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                      Station parameters                      //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * \brief Station longitude [decimal degrees]
     */
    double longitude;

    /**
     * \brief Station latitude [decimal degrees]
     */
    double latitude;

    /**
     * \brief Station altitude [metres]
     */
    double altitude;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                     Detection parameters                     //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Number of frames to buffer for head of each detection, i.e.
     * before the event started.
     */
    unsigned int detection_head;

    /**
     * @brief Number of frames to buffer for tail of each detection, i.e.
     * after the event has ceased.
     */
    unsigned int detection_tail;

    /**
     * @brief Difference between the digital levels of a pixel between frames that indicate
     * a significant change, i.e. one that counts towards an event trigger.
     */
    unsigned int pixel_difference_threshold;

    /**
     * @brief Number of significantly changed pixels required to trigger an event detection.
     */
    unsigned int n_changed_pixels_for_trigger;




    MeteorCaptureState() {
        bufferinfo = new v4l2_buffer();
        memset(bufferinfo, 0, sizeof(*bufferinfo));

        format = new v4l2_format();
        memset(format, 0, sizeof(*format));

        bufrequest = new v4l2_requestbuffers();
        memset(bufrequest, 0, sizeof(*bufrequest));
    }

    ~MeteorCaptureState() {
        delete bufferinfo;
        delete format;
        delete bufrequest;
    }
};

#endif // METEORCAPTURESTATE_H
