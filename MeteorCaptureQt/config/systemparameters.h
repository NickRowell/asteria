#ifndef SYSTEMPARAMETERS_H
#define SYSTEMPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameterdir.h"

#include "infra/meteorcapturestate.h"

class SystemParameters : public ConfigParameterFamily {

public:

    SystemParameters(MeteorCaptureState * state) : ConfigParameterFamily("System", 2) {

        parameters = new ConfigParameter*[numPar];

        // Config/calibration directory
        // Directory for saving video clips
        // Path to the reference star catalogue
        // Path to any maps, JPL ephemeris etc
        // ...?
        parameters[0] = new ParameterDir("configDir", "Configuration directory", "", &(state->configDirPath), true, false, true);
        parameters[1] = new ParameterDir("videoDir", "Video directory", "", &(state->videoDirPath), true, false, true);
//        parameters[2] = new ParameterDir("Reference star catalogue file", &(state->refStarCataloguePath), true, true, true);
//        parameters[3] = new ParameterDir("JPL ephemeris file", &(state->jplEphemerisPath), true, true, true);
    }

};

#endif
