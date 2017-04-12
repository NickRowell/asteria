#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include "configparameter.h"
#include "parameterdouble.h"
#include "configparameterfamily.h"

class MeteorCaptureState;

class ConfigStore {

public:

    ConfigStore(MeteorCaptureState * state);

    ~ConfigStore();

    unsigned int numFamilies;

    ConfigParameterFamily ** families;

    void saveToFile(string &path);

    void loadFromFile(string &path);

};

#endif // CONFIGSTORE_H
