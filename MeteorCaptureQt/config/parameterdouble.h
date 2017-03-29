#ifndef PARAMETERDOUBLE_H
#define PARAMETERDOUBLE_H

#include <string>     // std::string, std::stod
#include "configparameter.h"

class ParameterDouble : public ConfigParameter
{

public:

    ParameterDouble(const string key, const string title, const string units, double * const data, const double &lower, const double &upper);

    double * const data;

    double lower;

    double upper;

    void validate(const string stringRep);

};

#endif // PARAMETERDOUBLE_H
