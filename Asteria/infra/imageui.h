#ifndef IMAGEUI_H
#define IMAGEUI_H

#include "infra/image.h"

#include <iostream>

/**
 * @brief Represents an image with 32-bit pixels; these are especially useful for representing
 * RGBA colour images with 8-bit channels.
 */
class Imageui : public Image<unsigned int>
{

public:

    Imageui();
    Imageui(const Imageui& copyme);
    Imageui(unsigned int &width, unsigned int &height);
    Imageui(unsigned int &width, unsigned int &height, unsigned int val);
    ~Imageui();

    void writeToStream(std::ostream &output) const;

    void readFromStream(std::istream &input);

};

#endif // IMAGEUI_H
