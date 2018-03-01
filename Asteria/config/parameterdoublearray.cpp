#include "parameterdoublearray.h"

#include "util/ioutil.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <QDebug>

ParameterDoubleArray::ParameterDoubleArray(const string key, const string title, const string units, const ParameterValidator * validator, unsigned int nEls, double *dataIn[])
    : ConfigParameterBase(key, title, units, validator), nEls(nEls), data(new double*[nEls])
{
    for(unsigned int i=0; i<nEls; i++) {
        data[i] = dataIn[i];
    }
}

ParameterDoubleArray::~ParameterDoubleArray() {
    delete data;
}

void ParameterDoubleArray::parseAndValidate(const string stringRep) {

    value = stringRep;

    std::ostringstream strs;

    // Tokenize the string; split on white space
    std::vector<std::string> x = IoUtil::split(stringRep, ' ');

    // Check we found the right amount of elements:
    if(x.size() != nEls) {
        isValid = INVALID;
        strs << "Expected " << nEls << " elements, found " << x.size() << ":";
        for(unsigned int i=0; i<x.size(); i++) {
            strs << " \'" << x[i] << "\' ";
        }
        message = strs.str();
        return;
    }

    // Parse each element in turn
    double * vals = new double[x.size()];

    for(unsigned int i=0; i<x.size(); i++) {

        // First check that the string can be parsed to a double
        try {
            vals[i] = std::stod(x[i]);
        }
        catch(std::exception& e) {
            isValid = INVALID;
            strs << "Couldn't parse element " << i << " from " << x[i];
            message = strs.str();
            return;
        }
    }

    // Now perform any additional tests on the value itself
    if(!validator->validate(vals, strs)) {
        message = strs.str();
        isValid = INVALID;
        return;
    }

    // All good: update the value of the state fields
    for(unsigned int i=0; i<x.size(); i++) {
        *data[i] = vals[i];
    }
    isValid = VALID;
    message = "";
    return;
}
