#ifndef PARAMETERDIR_H
#define PARAMETERDIR_H

#include "configparameterbase.h"

class ParameterDir : public ConfigParameterBase
{

public:

    ParameterDir(const string key, const string title, const string units, const ParameterValidator * validator, string * const data);

    // Pointer to the corresponding parameter in the state object, which is set whenever the parameter is
    // read from the GUI/config file and validated successfully
    string * const data;

    void parseAndValidate(const string path);

};

#endif // PARAMETERDOUBLE_H
