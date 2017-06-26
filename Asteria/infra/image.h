#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <iostream>

class Image
{

public:

    Image();
    Image(const Image& copyme);
    Image(unsigned int &width, unsigned int &height);
    Image(unsigned int &width, unsigned int &height, unsigned char val);
    ~Image();

    unsigned int width;
    unsigned int height;

    // Raw (greyscale) image data for analysis
    std::vector<unsigned char> rawImage;

    /**
     * @brief epochTimeUs
     * Records the epoch time in microseconds, i.e. the time elapsed since 1970-01-01T00:00:00Z,
     * of the time the first byte of image data was captured.
     */
    long long epochTimeUs;

    // Remaining fields are transient (not serialised)

    // Optional RGBA version of the image with annotations, for display.
    // Not to be computed if it's not being displayed in real time.
    std::vector<unsigned int> annotatedImage;



    // Some statistics of the image acquisition at the time the image was captured
    double fps;
    unsigned int droppedFrames;
    unsigned int totalFrames;

    friend std::ostream &operator<<(std::ostream &output, const Image &image);

    friend std::istream &operator>>(std::istream  &input, Image &image);

    /**
     * Function used to aid sorting of a vector of Images into ascending order of capture time
     * using std::sort(vecOfImages.begin(), vecOfImages.end());
     *
     * @brief operator <
     * @param image
     * @return
     */
    bool operator < (const Image& image) const;

};

#endif // IMAGE_H
