#include "parameteruint.h"

#include <iostream>
#include <sstream>
#include <QDebug>

ParameterUint::ParameterUint(const string key, const string title, const string units, const ParameterValidator * validator, unsigned int * const data)
    : ConfigParameterBase(key, title, units, validator), data(data)
{

}

void ParameterUint::parseAndValidate(const string stringRep) {

    value = stringRep;

    // First check that the string can be parsed to an int
    unsigned int parsed;
    try {
        parsed = std::stoi(stringRep);
    }
    catch(std::exception& e) {
        isValid = INVALID;
        message = "Couldn't parse "+this->key+" from string \'" + stringRep + "\'";
        return;
    }

    // Now perform any additional tests on the value itself
    std::ostringstream strs;

    if(!validator->validate(&parsed, strs)) {
        message = strs.str();
        isValid = INVALID;
        return;
    }

    // All good: update the value of the state field
    (*data) = parsed;
    isValid = VALID;
    message = "";
    return;
}
