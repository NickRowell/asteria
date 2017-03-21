#ifndef CONFIGPARAMETER_H
#define CONFIGPARAMETER_H

#include <string>

using namespace std;

/**
 * @brief The ConfigParameter class
 * Base class for all configuration parameter types. Each subclass must provide an implementation
 * of the validate(...) method and provide meaningful error messages if it fails to parse a
 * value from the string. Each subclass must also contain a field that stores the parsed, converted
 * value of the parameter.
 */
class ConfigParameter
{

protected:
    ConfigParameter(const string key);

public:

    // String representation of the parameter name
    const string key;

    // Flag indicating if the value is valid
    bool isValid;

    // Contains error message explaining why the value cannot be parsed from the string
    string message;

    // Abstract validation method that all parameters must override
    virtual void validate(const string stringRep) =0;

};

#endif // CONFIGPARAMETER_H
