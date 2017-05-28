#ifndef PARAMETERDOUBLE_H
#define PARAMETERDOUBLE_H

#include <string>     // std::string, std::stod
#include "configparameter.h"

class ParameterDouble : public ConfigParameterBase
{

public:

    ParameterDouble(const string key, const string title, const string units, const ParameterValidator * validator, double * const data);

    // Pointer to the corresponding parameter in the state object, which is set whenever the parameter is
    // read from the GUI/config file and validated successfully
    double * const data;

    void parseAndValidate(const string stringRep);

};

#endif // PARAMETERDOUBLE_H
