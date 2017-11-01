#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <memory>

/**
 * @brief The base template class for types representing images with samples of different data types. Primarily this
 * is useful for representing images captured from cameras, for which the samples are positive integers in the
 * [0:255] range, and images produced for calibration purposes for which the samples are often floating point
 * numbers and can be negative.
 */
template<class T> class Image
{

public:

    Image() : width(0), height(0), epochTimeUs(0ll), rawImage(0) {
        // Nothing to do
    }

    Image(const Image& copyme) : width(copyme.width), height(copyme.height), epochTimeUs(copyme.epochTimeUs), rawImage(copyme.rawImage) {
        // Nothing to do
    }

    Image(unsigned int &width, unsigned int &height) : width(width), height(height), epochTimeUs(0ll), rawImage(width*height) {
        // Nothing to do
    }

    Image(unsigned int &width, unsigned int &height, T val) : width(width), height(height), epochTimeUs(0ll), rawImage(width*height, val) {
        // Nothing to do
    }

    ~Image() {
        rawImage.clear();
    }

    /**
     * @brief The image width [pixels]
     */
    unsigned int width;

    /**
     * @brief The image height [pixels]
     */
    unsigned int height;

    /**
     * @brief Records the epoch time in microseconds. This is the time elapsed since 1970-01-01T00:00:00Z
     * of the time the first byte of image data was captured.
     */
    long long epochTimeUs;

    /**
     * @brief Raw image data in a 1D flattened vector.
     */
    std::vector<T> rawImage;

    /**
     * @brief Serialises the Image to a ostream.
     * @param output
     *  The ostream to write to.
     */
    virtual void writeToStream(std::ostream &output) const =0;

    /**
     * @brief Deserialises the Image from an istream.
     * @param input
     *  The istream to read from.
     */
    virtual void readFromStream(std::istream &input) =0;

    /**
     * @brief Function used to aid sorting of a vector of Images into ascending order of capture time
     * using std::sort(vecOfImages.begin(), vecOfImages.end());
     *
     * @param image
     *  The image to compare to this one.
     * @return
     *  True if this image was captured at an earlier time than the given one.
     */
    bool operator < (const Image& image) const {
        return (epochTimeUs < image.epochTimeUs);
    }

    /**
     * @brief Used to sort a vector of pointers to Images into ascending order of capture time.
     * @param a
     *  The first image.
     * @param b
     *  The second image.
     * @return
     *  True if the first image is earlier than the second image.
     */
    static bool comparePtrToImage(std::shared_ptr<Image> a, std::shared_ptr<Image> b) {
        return (*a < *b);
    }

    friend std::ostream &operator<<(std::ostream &output, const Image &image) {
        image.writeToStream(output);
        return output;
    }

    friend std::istream &operator>>(std::istream  &input, Image &image) {
        image.readFromStream(input);
        return input;
    }

};


#endif // IMAGE_H
