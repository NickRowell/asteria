#include "image.h"

Image::Image() {

}

Image::Image(const Image& copyme) : width(copyme.width), height(copyme.height) {
    pixelData = copyme.pixelData;
}

Image::Image(unsigned int width, unsigned int height) : width(width), height(height)
{

}

Image::~Image() {

}
