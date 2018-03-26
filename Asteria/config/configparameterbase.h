#ifndef CONFIGPARAMETER_H
#define CONFIGPARAMETER_H

#include "config/validate.h"

#include <string>
#include <vector>

#include <QWidget>

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
 * @brief The ConfigParameterBase class
 * Non-templated base class so we can use vectors of pointers to ConfigParameterBase
 */
class ConfigParameterBase
{

protected:
    ConfigParameterBase(const string key, const string title, const string units, const ParameterValidator * validator) : key(key), title(title), units(units), validator(validator) {

    }

public:

    // Key used to identify the parameter in config file etc; no spaces etc
    const string key;

    // Descriptive name of the parameter
    const string title;

    // Descriptive name of the units, if appropriate to the parameter
    const string units;

    // Validator for the parameter value
    const ParameterValidator * validator;

    // String representation of the parameter value, from which any valid parameter value was parsed
    string value;

    // Flag indicating if the value is valid
    ParameterValidity isValid;

    // Contains error message explaining why the value cannot be parsed from the string
    string message;

    /**
     * @brief A pointer to a QWidget that can be inserted into a GUI to allow the user to configure the
     * value of the parameter. The type of widget that is actually used will vary depending on the type
     * of parameter.
     */
    QWidget * entryField;

    /**
     * @brief Reads the value of the parameter from the entryField QWidget, parses and validates it.
     */
//    virtual void parseAndValidate() =0;

    // Abstract validation method that all parameters must override
    // The =0 syntax makes this a 'pure virtual' function that MUST be overridden in derived classes.
    virtual void parseAndValidate(const string stringRep) =0;
};

#endif // CONFIGPARAMETER_H
