#ifndef CONFIGPARAMETERFAMILY_H
#define CONFIGPARAMETERFAMILY_H

#include <string>

class ConfigParameter;

using namespace std;


class ConfigParameterFamily
{

public:

    ConfigParameterFamily(const string title, const unsigned int numPar);

    const string title;

    unsigned int numPar;

    ConfigParameter ** parameters;
};

#endif // CONFIGPARAMETERFAMILY_H
