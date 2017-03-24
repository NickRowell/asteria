#ifndef STATIONPARAMETERS_H
#define STATIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameterdouble.h"
#include "infra/meteorcapturestate.h"

class StationParameters : public ConfigParameterFamily {

public:

    StationParameters(MeteorCaptureState * state) : ConfigParameterFamily("Station", 3) {
        parameters = new ConfigParameter*[numPar];
        parameters[0] = new ParameterDouble("Longitude [deg]", &(state->longitude), 0.0, 360.0);
        parameters[1] = new ParameterDouble("Latitude [deg]", &(state->latitude), -90.0, 90.0);
        parameters[2] = new ParameterDouble("Altitude [m]", &(state->altitude), -100.0, 5000.0);
    }

};

#endif // STATIONPARAMETERS_H
