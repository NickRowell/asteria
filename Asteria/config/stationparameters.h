#ifndef STATIONPARAMETERS_H
#define STATIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/singleparameter.h"
#include "config/validate.h"
#include "infra/asteriastate.h"

class StationParameters : public ConfigParameterFamily {

public:

    StationParameters(AsteriaState * state) : ConfigParameterFamily("Station", 3) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
        validators[0] = new ValidateWithinLimits<double>(-180.0, 180.0);
        validators[1] = new ValidateWithinLimits<double>(-90.0, 90.0);
        validators[2] = new ValidateWithinLimits<double>(-100.0, 5000.0);

        // Create parameters
        parameters[0] = new SingleParameter<double>("longitude", "Longitude (+ve E)", "deg", validators[0], &(state->longitude));
        parameters[1] = new SingleParameter<double>("latitude", "Latitude", "deg", validators[1], &(state->latitude));
        parameters[2] = new SingleParameter<double>("altitude", "Altitude", "m", validators[2], &(state->altitude));
    }

};

#endif // STATIONPARAMETERS_H
