#ifndef PARAMETERDOUBLE_H
#define PARAMETERDOUBLE_H

#include <string>     // std::string, std::stod
#include "configparameter.h"

class ParameterDouble : public ConfigParameter
{

public:

    ParameterDouble(const string key, double * const data, const double &lower, const double &upper);

    double lower;

    double upper;

    double * const data;

    void validate(const string stringRep);

};

#endif // PARAMETERDOUBLE_H
