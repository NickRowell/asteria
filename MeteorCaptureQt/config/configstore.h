#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include "configparameter.h"
#include "parameterdouble.h"
#include "configparameterfamily.h"

class MeteorCaptureState;

class ConfigStore {

public:

    ConfigStore(MeteorCaptureState * state);

    ConfigParameterFamily ** families;
    unsigned int numFamilies;

};

#endif // CONFIGSTORE_H
