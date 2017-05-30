#ifndef DETECTIONPARAMETERS_H
#define DETECTIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameteruint.h"
#include "infra/asteriastate.h"

#include <QDebug>

class DetectionParameters : public ConfigParameterFamily {

public:

    DetectionParameters(AsteriaState * state) : ConfigParameterFamily("Detection", 4) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
        validators[0] = new ValidateWithinLimits<unsigned int>(0u, 100u);
        validators[1] = new ValidateWithinLimits<unsigned int>(0u, 100u);
        validators[2] = new ValidateWithinLimits<unsigned int>(1u, 255u);
        validators[3] = new ValidateWithinLimits<unsigned int>(1u, 1000u);

        // Create parameters
        parameters[0] = new ParameterUint("detection_head", "Detection head", "frames", validators[0], &(state->detection_head));
        parameters[1] = new ParameterUint("detection_tail", "Detection tail", "frames", validators[1], &(state->detection_tail));
        parameters[2] = new ParameterUint("pixel_difference_threshold", "Pixel difference threshold", "ADU", validators[2], &(state->pixel_difference_threshold));
        parameters[3] = new ParameterUint("n_changed_pixels_for_trigger", "Number of changed pixels that triggers an event", "pixels", validators[3], &(state->n_changed_pixels_for_trigger));
    }
};

#endif
