#include "parameterdir.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <QDebug>

ParameterDir::ParameterDir(const string key, const string title, const string units, const ParameterValidator *validator, string * const data)
    : ConfigParameterBase(key, title, units, validator), data(data)
{

}

void ParameterDir::parseAndValidate(const string path) {

    value = path;

    // Now perform any additional tests on the value itself
    std::ostringstream strs;

    if(!validator->validate(&value, strs)) {
        message = strs.str();
        isValid = INVALID;
        return;
    }

    // If we've reached this point then the directory does exist and we have the necessary
    // permissions, even if we had to create it ourselves.
    message = "";
    (*data) = path;
    isValid = VALID;
    return;
}
