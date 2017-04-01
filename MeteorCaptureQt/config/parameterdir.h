#ifndef PARAMETERDIR_H
#define PARAMETERDIR_H

#include "configparameter.h"

class ParameterDir : public ConfigParameter
{

public:

    ParameterDir(const string key, const string title, const string units, string * const data, const bool &requireWritePermission,
                 const bool &requireExists, const bool &createIfNotExists);

    string * const data;

    bool requireWritePermission;

    bool requireExists;

    bool createIfNotExists;

    void validate(const string path);

};

#endif // PARAMETERDOUBLE_H
