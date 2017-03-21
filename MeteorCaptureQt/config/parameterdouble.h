#ifndef PARAMETERDOUBLE_H
#define PARAMETERDOUBLE_H

#include <string>     // std::string, std::stod
#include "configparameter.h"

class ParameterDouble : public ConfigParameter
{

public:

    ParameterDouble(const string key, const double &lower, const double &upper);

    double lower;

    double upper;

    double data;

    void validate(const string stringRep);

};

#endif // PARAMETERDOUBLE_H
