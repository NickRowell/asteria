#ifndef CAMERAPARAMETERS_H
#define CAMERAPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameteruintarray.h"
#include "config/parameterdoublearray.h"
#include "config/singleparameter.h"
#include "infra/asteriastate.h"

class CameraParameters : public ConfigParameterFamily {

public:

    CameraParameters(AsteriaState * state) : ConfigParameterFamily("Camera", 6) {

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
        validators[1] = new ValidateWithinLimits<double>(0.0, 300.0);
        validators[2] = new ValidateWithinLimits<double>(0.0, 30.0);
        validators[3] = new ValidateWithinLimits<double>(0.0, 360.0);
        validators[4] = new ValidateWithinLimits<double>(-90.0, 90.0);
        validators[5] = new ValidateWithinLimits<double>(-180.0, 180.0);


        unsigned int * image_width_height[2] = {&(state->width), &(state->height)};
        double * pixel_width_height[2] = {&(state->pixel_width), &(state->pixel_height)};

        // Create parameters
        parameters[0] = new ParameterUintArray("image_width_height", "Image Width and Height", "pixels", validators[0], 2u, image_width_height);
        parameters[1] = new SingleParameter<double>("focal_length", "Focal Length", "mm", validators[1], &(state->focal_length));
        parameters[2] = new ParameterDoubleArray("pixel_width_height", "Pixel Width and Height", "um", validators[2], 2u, pixel_width_height);
        parameters[3] = new SingleParameter<double>("azimuth", "Azimuth (east of north)", "deg", validators[3], &(state->azimuth));
        parameters[4] = new SingleParameter<double>("elevation", "Elevation (from horizon)", "deg", validators[4], &(state->elevation));
        parameters[5] = new SingleParameter<double>("roll", "Roll (clockwise around boresight)", "deg", validators[5], &(state->roll));

    }
};

#endif
