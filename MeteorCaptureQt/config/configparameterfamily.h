#ifndef CONFIGPARAMETERFAMILY_H
#define CONFIGPARAMETERFAMILY_H

#include <string>

class ConfigParameterBase;
class ParameterValidator;

using namespace std;


class ConfigParameterFamily
{

public:

    ConfigParameterFamily(const string title, const unsigned int numPar);

    ~ConfigParameterFamily();

    const string title;

    unsigned int numPar;

    ConfigParameterBase ** parameters;

    ParameterValidator ** validators;
};

#endif // CONFIGPARAMETERFAMILY_H
