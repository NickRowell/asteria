#ifndef SYSTEMPARAMETERS_H
#define SYSTEMPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parametersingle.h"
#include "infra/asteriastate.h"

class SystemParameters : public ConfigParameterFamily {

public:

    SystemParameters(AsteriaState * state) : ConfigParameterFamily("System", 4) {

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
        validators[2] = new ValidatePath(false, true, false, true);
        validators[3] = new ValidatePath(true, false, true, false);

        // Create parameters
        parameters[0] = new ParameterSingle<string>("configDir", "Configuration directory", "", validators[0], &(state->configDirPath));
        parameters[1] = new ParameterSingle<string>("calibrationDir", "Calibration directory", "", validators[1], &(state->calibrationDirPath));
        parameters[2] = new ParameterSingle<string>("videoDir", "Video directory", "", validators[2], &(state->videoDirPath));
        parameters[3] = new ParameterSingle<string>("refStarCatPath", "Reference star catalogue", "", validators[3], &(state->refStarCataloguePath));

//        parameters[3] = new SingleParameter<string>("JPL ephemeris file", &(state->jplEphemerisPath));
    }

};

#endif
