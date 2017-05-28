#include "parameteruintarray.h"

#include "util/IOUtil.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <QDebug>

ParameterUintArray::ParameterUintArray(const string key, const string title, const string units, const ParameterValidator * validator, unsigned int nEls, unsigned int *dataIn[])
    : ConfigParameterBase(key, title, units, validator), nEls(nEls), data(new unsigned int*[nEls])
{
    for(unsigned int i=0; i<nEls; i++) {
        data[i] = dataIn[i];
    }
}

ParameterUintArray::~ParameterUintArray() {
    delete data;
}

void ParameterUintArray::parseAndValidate(const string stringRep) {

    value = stringRep;

    std::ostringstream strs;

    // Tokenize the string; split on white space
    std::vector<std::string> x = split(stringRep, ' ');

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
    unsigned int * vals = new unsigned int[x.size()];

    for(unsigned int i=0; i<x.size(); i++) {

        // First check that the string can be parsed to an unsigned int
        try {
            vals[i] = std::stoi(x[i]);
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
