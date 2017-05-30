#ifndef CAMERAPARAMETERS_H
#define CAMERAPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameteruintarray.h"
#include "infra/asteriastate.h"

#include <QDebug>

class CameraParameters : public ConfigParameterFamily {

public:

    CameraParameters(AsteriaState * state) : ConfigParameterFamily("Camera", 3) {

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
        validators[1] = new ValidateWithinLimits<double>(0.0, 360.0);
        validators[2] = new ValidateWithinLimits<double>(-90.0, 90.0);

        unsigned int * vals[2] = {&(state->width), &(state->height)};

        // Create parameters
        parameters[0] = new ParameterUintArray("image_width_height", "Image Width and Height", "pixels", validators[0], 2u, vals);
        parameters[1] = new ParameterDouble("azimuth", "Azimuth", "deg", validators[1], &(state->azimuth));
        parameters[2] = new ParameterDouble("elevation", "Elevation", "deg", validators[2], &(state->elevation));

    }
};

#endif
