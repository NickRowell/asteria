#include "configparameterfamily.h"

#include <QDebug>


ConfigParameterFamily::ConfigParameterFamily(const string title, unsigned int numPar) : title(title), numPar(numPar) {}

ConfigParameterFamily::~ConfigParameterFamily() {

    // Delete parameters and validators
    for(unsigned int parOff = 0; parOff < numPar; parOff++) {
        delete parameters[parOff];
        delete validators[parOff];
    }
    // Delete arrays
    delete [] parameters;
    delete [] validators;
}
