#ifndef PARAMETERINTARRAY_H
#define PARAMETERINTARRAY_H

#include <string>
#include "configparameterbase.h"

class ParameterUintArray : public ConfigParameterBase
{

public:

    ParameterUintArray(const string key, const string title, const string units, const ParameterValidator * validator, unsigned int nEls, unsigned int * data[]);

    ~ParameterUintArray();

    // Number of elements in the array; this is the number of values we expect to parse form the string and
    // the number of pointers contained in the pointer array.
    unsigned int nEls;

    // Array of pointers to the corresponding parameters in the state object, which are set whenever the parameters are
    // read from the GUI/config file and validated successfully
    unsigned int ** const data;

    void parseAndValidate(const string stringRep);

};

#endif // PARAMETERINTARRAY_H
