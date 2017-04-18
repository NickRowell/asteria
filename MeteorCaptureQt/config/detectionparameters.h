#ifndef DETECTIONPARAMETERS_H
#define DETECTIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameteruint.h"
#include "infra/meteorcapturestate.h"

#include <QDebug>

class DetectionParameters : public ConfigParameterFamily {

public:

    DetectionParameters(MeteorCaptureState * state) : ConfigParameterFamily("Detection", 1) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
        validators[0] = new ValidateWithinLimits<unsigned int>(0u, 100u);

        // Create parameters
        parameters[0] = new ParameterUint("detection_head", "Detection head", "frames", validators[0], &(state->detection_head));

    }
};

#endif
