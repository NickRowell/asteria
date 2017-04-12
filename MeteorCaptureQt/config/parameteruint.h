#ifndef PARAMETERINT_H
#define PARAMETERINT_H

#include <string>     // std::string, std::stod
#include "configparameter.h"

class ParameterUint : public ConfigParameterBase
{

public:

    ParameterUint(const string key, const string title, const string units, const ParameterValidator * validator, unsigned int * const data);

    // Pointer to the corresponding parameter in the state object, which is set whenever the parameter is
    // read from the GUI/config file and validated successfully
    unsigned int * const data;

    void parseAndValidate(const string stringRep);

};

#endif // PARAMETERINT_H
