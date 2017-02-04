#ifndef METEORCAPTURESTATE_H
#define METEORCAPTURESTATE_H

#include <string>     // provides string

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

    MeteorCaptureState() {}
};

#endif // METEORCAPTURESTATE_H
