#include "infra/referencestar.h"
#include "util/ioutil.h"
#include "util/mathutil.h"

#include <iostream>
#include <fstream>

#include <QDebug>

ReferenceStar::ReferenceStar() : ra(0.0), dec(0.0), mag(0.0) {

}

ReferenceStar::ReferenceStar(const ReferenceStar& copyme) : ra(copyme.ra), dec(copyme.dec), mag(copyme.mag), i(copyme.i), j(copyme.j), r(copyme.r) {

}

ReferenceStar::ReferenceStar(const double &ra, const double &dec, const double &mag) : ra(ra), dec(dec), mag(mag) {

}

ReferenceStar& ReferenceStar::operator=(const ReferenceStar& copyme) {
    ra = copyme.ra;
    dec = copyme.dec;
    mag = copyme.mag;
    i = copyme.i;
    j = copyme.j;
    r = copyme.r;

    return *this;
}


std::vector<ReferenceStar> ReferenceStar::loadCatalogue(std::string &path) {

    std::vector<ReferenceStar> catalogue;

    std::ifstream myfile(path);
    std::string line;
    unsigned int counter = 0;
    if (myfile.is_open())
    {
        while ( getline (myfile, line) )
        {
            counter++;
            // Ignore comments
            if (line[0] != '#' )
            {
                // Tokenise the string
                // TODO: split on any amount of whitespace
                std::vector<std::string> x = IoUtil::split(line, '\t');

                // Check that we got preciely three tokens (ra, dec, magnitude):
                if(x.size()==3) {
                    std::string raStr = x[0];
                    std::string decStr = x[1];
                    std::string magStr = x[2];

                    // Parse the ra, dec and mag
                    double ra, dec, mag;
                    try {
                        ra  = std::stod(raStr);
                    }
                    catch(std::exception& e) {
//                        message = "Line " + counter + ": Couldn't parse RA from string \'" + raStr + "\'";
//                        return false;
                        continue;
                    }
                    try {
                        dec = std::stod(decStr);
                    }
                    catch(std::exception& e) {
//                        message = "Line " + counter + ": Couldn't parse Dec from string \'" + decStr + "\'";
//                        return false;
                        continue;
                    }
                    try {
                        mag = std::stod(magStr);
                    }
                    catch(std::exception& e) {
//                        message = "Line " + counter + ": Couldn't parse RA from string \'" + magStr + "\'";
//                        return false;
                        continue;
                    }

                    ReferenceStar star(MathUtil::toRadians(ra), MathUtil::toRadians(dec), mag);
                    catalogue.push_back(star);
                }
                else {
                    // Log error
//                    message = "Line " + counter + ": did not find 3 values";
//                    return false;
                }
            }
        }
        myfile.close();
    }

    return catalogue;
}
