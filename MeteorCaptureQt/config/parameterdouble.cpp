#include "parameterdouble.h"

#include <iostream>
#include <sstream>
#include <QDebug>

ParameterDouble::ParameterDouble(const string key, const string title, const string units, const ParameterValidator * validator, double * const data)
    : ConfigParameterBase(key, title, units, validator), data(data)
{

}

void ParameterDouble::parseAndValidate(const string stringRep) {

    value = stringRep;

    // First check that the string can be parsed to a double
    double parsed;
    try {
        parsed = std::stod(stringRep);
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
