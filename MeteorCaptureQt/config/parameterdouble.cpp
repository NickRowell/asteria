#include "parameterdouble.h"

#include <iostream>
#include <sstream>
#include <QDebug>

ParameterDouble::ParameterDouble(const string key, const double &lower, const double &upper) : ConfigParameter(key), lower(lower), upper(upper)
{

}

void ParameterDouble::validate(const string stringRep) {

    // First check that the string can be parsed to a double
    try {
        data = std::stod(stringRep);
    }
    catch(std::exception& e) {
        this->isValid = false;
        this->message = "Couldn't parse "+this->key+" from string " +stringRep;
        return;
    }

    // Now perform any additional tests on the value itself
    if(data <= lower || data >= upper) {
        std::ostringstream strs;
        strs << "Parameter " << this->key << " lies outside allowed range [" << lower << ":" << upper << "]";
        this->isValid = false;
        this->message = strs.str();
        return;
    }

    this->isValid = true;
    this->message = "";
    return;
}
