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

    /**
     * @brief epochTimeUs
     * Records the epoch time in microseconds, i.e. the time elapsed since 1970-01-01T00:00:00Z,
     * of the time the first byte of image data was captured.
     */
    long epochTimeUs;




};

#endif // IMAGE_H
