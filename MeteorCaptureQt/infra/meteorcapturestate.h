#ifndef METEORCAPTURESTATE_H
#define METEORCAPTURESTATE_H

#include <string>     // provides string

using namespace std;

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

    // Station parameters

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

    // Camera parameters

    // Detection parameters

    // Analysis parameters


    MeteorCaptureState() {
    }

    ~MeteorCaptureState() {
    }
};

#endif // METEORCAPTURESTATE_H
