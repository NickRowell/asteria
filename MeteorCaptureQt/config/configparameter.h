#ifndef CONFIGPARAMETER_H
#define CONFIGPARAMETER_H

#include <string>

using namespace std;

/**
 * @brief The ParameterValidity enum
 * VALID: the parameter parsed without a problem.
 * INVALID: parameter could not be parsed.
 * WARNING: the parameter could be parsed, but might have had it's value adjusted
 * or may cause some unexpected behaviour that the user should be alerted to.
 */
enum ParameterValidity{VALID, INVALID, WARNING};

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
    ConfigParameter(const string key, const string title, const string units) : key(key), title(title), units(units) {

    }

public:

    // Key used to enumerate the parameter; no spaces etc
    const string key;

    // Descriptive name of the parameter
    const string title;

    // Descriptive name of the units
    const string units;

    // String representation of the parameter value, from which any valid parameter value was parsed
    string value;

    // Flag indicating if the value is valid
    ParameterValidity isValid;

    // Contains error message explaining why the value cannot be parsed from the string
    string message;

    // Abstract validation method that all parameters must override
    virtual void validate(const string stringRep) =0;
};

#endif // CONFIGPARAMETER_H
