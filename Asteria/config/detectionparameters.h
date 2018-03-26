#ifndef DETECTIONPARAMETERS_H
#define DETECTIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/singleparameter.h"
#include "infra/asteriastate.h"

class DetectionParameters : public ConfigParameterFamily {

public:

    DetectionParameters(AsteriaState * state) : ConfigParameterFamily("Detection", 5) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
        validators[0] = new ValidateWithinLimits<unsigned int>(0u, 100u);
        validators[1] = new ValidateWithinLimits<unsigned int>(0u, 100u);
        validators[2] = new ValidateWithinLimits<double>(0.0, 2.0);
        validators[3] = new ValidateWithinLimits<unsigned int>(1u, 2550u);
        validators[4] = new ValidateWithinLimits<unsigned int>(1u, 100000u);

        // Create parameters
        parameters[0] = new SingleParameter<unsigned int>("detection_head", "Detection head", "frames", validators[0], &(state->detection_head));
        parameters[1] = new SingleParameter<unsigned int>("detection_tail", "Detection tail", "frames", validators[1], &(state->detection_tail));
        parameters[2] = new SingleParameter<double>("clip_max_length", "Maximum clip length, excluding head", "minutes", validators[2], &(state->clip_max_length));
        parameters[3] = new SingleParameter<unsigned int>("pixel_difference_threshold", "Pixel difference threshold", "ADU", validators[3], &(state->pixel_difference_threshold));
        parameters[4] = new SingleParameter<unsigned int>("n_changed_pixels_for_trigger", "Number of changed pixels that triggers an event", "pixels", validators[4], &(state->n_changed_pixels_for_trigger));
    }
};

#endif
