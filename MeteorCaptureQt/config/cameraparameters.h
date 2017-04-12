#ifndef CAMERAPARAMETERS_H
#define CAMERAPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameteruintarray.h"
#include "infra/meteorcapturestate.h"

#include <QDebug>

class CameraParameters : public ConfigParameterFamily {

public:

    CameraParameters(MeteorCaptureState * state) : ConfigParameterFamily("Camera", 1) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Image size
        // Lens focal length
        // Aperture size
        // Projection matrix
        // Azimuth
        // Elevation
        // Pixel depth (?)
        // Gain (?)
        // Exposure time (?)

        // Create validators for each parameter
        validators[0] = new ValidateImageSize(state);

        unsigned int * vals[2] = {&(state->width), &(state->height)};

        // Create parameters
        parameters[0] = new ParameterUintArray("image_width_height", "Image Width and Height", "pixels", validators[0], 2u, vals);
    }
};

#endif
