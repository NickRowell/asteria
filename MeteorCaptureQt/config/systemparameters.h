#ifndef SYSTEMPARAMETERS_H
#define SYSTEMPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameterdir.h"
#include "infra/meteorcapturestate.h"

#include <QDebug>

class SystemParameters : public ConfigParameterFamily {

public:

    SystemParameters(MeteorCaptureState * state) : ConfigParameterFamily("System", 2) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Config/calibration directory
        // Directory for saving video clips
        // Path to the reference star catalogue
        // Path to any maps, JPL ephemeris etc
        // ...?

        // Create validators for each parameter
        validators[0] = new ValidateDirectory(true, false, true);
        validators[1] = new ValidateDirectory(true, false, true);

        // Create parameters
        parameters[0] = new ParameterDir("configDir", "Configuration directory", "", validators[0], &(state->configDirPath));
        parameters[1] = new ParameterDir("videoDir", "Video directory", "", validators[1], &(state->videoDirPath));
//        parameters[2] = new ParameterDir("Reference star catalogue file", &(state->refStarCataloguePath));
//        parameters[3] = new ParameterDir("JPL ephemeris file", &(state->jplEphemerisPath));
    }

};

#endif
