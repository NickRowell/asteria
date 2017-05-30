#ifndef SYSTEMPARAMETERS_H
#define SYSTEMPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameterdir.h"
#include "infra/asteriastate.h"

#include <QDebug>

class SystemParameters : public ConfigParameterFamily {

public:

    SystemParameters(AsteriaState * state) : ConfigParameterFamily("System", 3) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Config/calibration directory
        // Directory for saving video clips
        // Path to the reference star catalogue
        // Path to any maps, JPL ephemeris etc
        // ...?

        // Create validators for each parameter
        validators[0] = new ValidatePath(false, true, true, false);
        validators[1] = new ValidatePath(false, true, false, true);
        validators[2] = new ValidatePath(true, false, true, false);

        // Create parameters
        parameters[0] = new ParameterDir("configDir", "Configuration directory", "", validators[0], &(state->configDirPath));
        parameters[1] = new ParameterDir("videoDir", "Video directory", "", validators[1], &(state->videoDirPath));
        parameters[2] = new ParameterDir("refStarCatPath", "Reference star catalogue", "", validators[2], &(state->refStarCataloguePath));

//        parameters[3] = new ParameterDir("JPL ephemeris file", &(state->jplEphemerisPath));
    }

};

#endif
