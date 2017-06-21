#ifndef CALIBRATIONPARAMETERS_H
#define CALIBRATIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameterdouble.h"
#include "config/parameteruint.h"
#include "infra/asteriastate.h"

#include <QDebug>

class CalibrationParameters : public ConfigParameterFamily {

public:

    CalibrationParameters(AsteriaState * state) : ConfigParameterFamily("Calibration", 2) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
        validators[0] = new ValidateWithinLimits<double>(0.0, 100.0);
        validators[1] = new ValidateWithinLimits<unsigned int>(0u, 100u);

        // Create parameters
        parameters[0] = new ParameterDouble("calibration_interval", "Calibration Interval", "minutes", validators[0], &(state->calibration_interval));
        parameters[1] = new ParameterUint("calibration_stack", "Number of frames used for calibration", "frames", validators[1], &(state->calibration_stack));
    }
};

#endif
