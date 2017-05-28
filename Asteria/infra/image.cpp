#include "image.h"

Image::Image() {

}

Image::Image(const Image& copyme) : width(copyme.width), height(copyme.height), rawImage(copyme.rawImage), annotatedImage(copyme.annotatedImage), epochTimeUs(copyme.epochTimeUs) {

}

Image::Image(unsigned int width, unsigned int height) : width(width), height(height), rawImage(width * height), annotatedImage(width * height)
{

}

Image::~Image() {

}
