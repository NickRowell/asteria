#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

class Image
{

public:

    Image();
    Image(const Image& copyme);
    Image(unsigned int width, unsigned int height);
    ~Image();


    std::vector<char> pixelData;

    unsigned int width;
    unsigned int height;




};

#endif // IMAGE_H
