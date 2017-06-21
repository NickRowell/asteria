#ifndef ANALYSISPARAMETERS_H
#define ANALYSISPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameterdouble.h"
#include "infra/asteriastate.h"

#include <QDebug>

class AnalysisParameters : public ConfigParameterFamily {

public:

    AnalysisParameters(AsteriaState * state) : ConfigParameterFamily("Analysis", 1) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
        validators[0] = new ValidateWithinLimits<double>(0.0, 100.0);

        // Create parameters
        parameters[0] = new ParameterDouble("linearity_threshold", "Linearity threshold", "pixels", validators[0], &(state->linearity_threshold));
    }
};

#endif
