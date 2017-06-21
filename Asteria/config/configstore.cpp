#include "config/configstore.h"
#include "configparameterfamily.h"
#include "config/systemparameters.h"
#include "config/stationparameters.h"
#include "config/cameraparameters.h"
#include "config/detectionparameters.h"
#include "config/analysisparameters.h"
#include "config/calibrationparameters.h"
#include "infra/asteriastate.h"
#include "util/IOUtil.h"

#include <vector>
#include <iostream>
#include <fstream>

#include <QDebug>

ConfigStore::ConfigStore(AsteriaState *state) {
    numFamilies = 6;
    families = new ConfigParameterFamily*[numFamilies];
    families[0] = new SystemParameters(state);
    families[1] = new StationParameters(state);
    families[2] = new CameraParameters(state);
    families[3] = new DetectionParameters(state);
    families[4] = new AnalysisParameters(state);
    families[5] = new CalibrationParameters(state);
}

ConfigStore::~ConfigStore() {
    for(unsigned int famOff = 0; famOff < numFamilies; famOff++) {
        delete families[famOff];
    }
    delete [] families;
}

void ConfigStore::saveToFile(string &path) {

    ofstream myfile(path);

    // Loop over parameter families
    for(unsigned int famOff = 0; famOff < numFamilies; famOff++) {

        // Get pointer to this family
        ConfigParameterFamily * fam = families[famOff];

        myfile << "# " << fam->title << " Parameters\n";

        string famPrefix = fam->title;

        // Loop over parameters within this family
        for(unsigned int parOff = 0; parOff < fam->numPar; parOff++) {
            // Get pointer to this parameter
            ConfigParameterBase * par = fam->parameters[parOff];
            // Write to file
            myfile << famPrefix << "." << par->key << "=" << par->value << "\n";
        }

        myfile << "\n";
    }
    myfile.close();
}

/**
 *
 * 1) Read the parameter family & title from the line
 * 2) Find the right parameter in the config store and set it's value
 *
 *
 * @brief ConfigStore::loadFromFile
 * @param path
 */
void ConfigStore::loadFromFile(string &path) {

    ifstream myfile(path);
    string line;
    if (myfile.is_open())
    {
        while ( getline (myfile, line) )
        {
            // Ignore comments
            if (line[0] != '#' )
            {
                // Tokenise the string
                std::vector<std::string> x = split(line, '=');

                // Check that we got preciely two tokens (parameter family/key and value):
                if(x.size()==2) {
                    string key = x[0];
                    string value = x[1];

                    // Now we need to determine which parameter we're dealing with from the key.
                    // This should be split into the parameter family & key
                    std::vector<std::string> y = split(key, '.');

                    // Check that we got precisely two tokens (parameter family and name):
                    if(y.size()==2) {
                        string familyName = y[0];
                        string parameterName = y[1];
                        // Look up the family
                        ConfigParameterFamily * fam = NULL;
                        for(unsigned int famOff = 0; famOff < numFamilies; famOff++) {
                            if(families[famOff]->title.compare(familyName) == 0) {
                                fam = families[famOff];
                                break;
                            }
                        }
                        if(fam != NULL) {
                            // Found the family; look up the parameter
                            ConfigParameterBase * par = NULL;
                            for(unsigned int parOff = 0; parOff < fam->numPar; parOff++) {
                                if(fam->parameters[parOff]->key.compare(parameterName) == 0) {
                                    par = fam->parameters[parOff];
                                    break;
                                }
                            }
                            if(par != NULL) {
                                // Got the parameter, got the value...
                                par->parseAndValidate(value);
                            }
                            else {
                                // Found parameter family but not the parameter
                                // Log error?
                            }
                        }
                        else {
                            // Couldn't find the parameter family
                            // Log error?
                        }

                    }
                    else {
                        // Log error?
                    }

                }
                else {
                    // Log error?
                }
            }
        }
        myfile.close();
    }
}
