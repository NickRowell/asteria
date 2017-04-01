#ifndef CAMERAPARAMETERS_H
#define CAMERAPARAMETERS_H

#include "config/configparameterfamily.h"
#include "infra/meteorcapturestate.h"

class CameraParameters : public ConfigParameterFamily {

public:

    CameraParameters(MeteorCaptureState * state) : ConfigParameterFamily("Camera", 3) {

        parameters = new ConfigParameter*[numPar];

        // Lens focal length
        // Image size (should this be set programatically depending on the selected camera?)
        // Aperture size
        // Projection matrix
        // Azimuth
        // Elevation
        // Pixel depth (?)
        // Gain (?)
        // Exposure time (?)

        parameters[0] = new ParameterDouble("longitude", "Longitude", "deg", &(state->longitude), 0.0, 360.0);
        parameters[1] = new ParameterDouble("latitude", "Latitude", "deg", &(state->latitude), -90.0, 90.0);
        parameters[2] = new ParameterDouble("altitude", "Altitude", "m", &(state->altitude), -100.0, 5000.0);
    }

};

#endif
