#include "parameterdouble.h"

#include <iostream>
#include <sstream>
#include <QDebug>

ParameterDouble::ParameterDouble(const string key, double * const data, const double &lower, const double &upper) : ConfigParameter(key), data(data), lower(lower), upper(upper)
{

}

void ParameterDouble::validate(const string stringRep) {

    // First check that the string can be parsed to a double
    double parsed;
    try {
        parsed = std::stod(stringRep);
    }
    catch(std::exception& e) {
        isValid = INVALID;
        message = "Couldn't parse "+this->key+" from string " +stringRep;
        return;
    }

    // Now perform any additional tests on the value itself
    if(parsed <= lower || parsed >= upper) {
        std::ostringstream strs;
        strs << "Parameter " << this->key << " (" << parsed << ")lies outside allowed range [" << lower << ":" << upper << "]";
        message = strs.str();
        isValid = INVALID;
        return;
    }

    // All good: update the value of the state field
    (*data) = parsed;
    value = stringRep;
    isValid = VALID;
    message = "";
    return;
}