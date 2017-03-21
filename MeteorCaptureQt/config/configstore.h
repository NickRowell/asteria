#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include "configparameter.h"
#include "parameterdouble.h"
#include "configparameterfamily.h"

class ConfigStore {

public:

    ConfigStore();

    ConfigParameterFamily ** families;
    unsigned int numFamilies;

};

#endif // CONFIGSTORE_H
