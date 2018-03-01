#ifndef IMAGED_H
#define IMAGED_H

#include "infra/image.h"

#include <iostream>

/**
 * @brief Represents an image with floating-point pixel values. This is useful for storing processed images
 * and quantities relevant to the camera calibration. See http://netpbm.sourceforge.net/doc/pfm.html for more information.
 */
class Imaged : public Image<double>
{

public:

    Imaged();
    Imaged(const Imaged& copyme);
    Imaged(unsigned int &width, unsigned int &height);
    Imaged(unsigned int &width, unsigned int &height, double val);
    ~Imaged();

    void writeToStream(std::ostream &output) const;

    void readFromStream(std::istream &input);
};

#endif // IMAGED_H
