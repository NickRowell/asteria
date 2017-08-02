#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <vector>
#include <iostream>
#include <linux/videodev2.h>

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

    /**
     * @brief field
     * The value of the v4l2_field enum representing the field order of the image.
     * Currently only the following types are supported:
     * V4L2_FIELD_NONE (progressive format; not interlaced)
     * V4L2_FIELD_INTERLACED (interleaved/interlaced format)
     * V4L2_FIELD_INTERLACED_TB (interleaved/interlaced format; top field is transmitted first)
     * V4L2_FIELD_INTERLACED_BT (interleaved/interlaced format; bottom field is transmitted first)
     */
    unsigned int field;

    /**
     * Positions of the edges of the bounding box that encloses the set of changed pixels for this image,
     * with respect to the earlier image. Outlier filtering is used so the bounding box may not contain
     * all of the changed pixels, but should provide a better localisation of the event trigger.
     */
    bool coarse_localisation_success;
    unsigned int bb_xmin;
    unsigned int bb_xmax;
    unsigned int bb_ymin;
    unsigned int bb_ymax;


    // Remaining fields are transient (not serialised)

    // Optional RGBA overlay image with annotations, for display.
    // Not to be computed if it's not being displayed in real time.
    std::vector<unsigned int> annotatedImage;

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

    /**
     * @brief comparePtrToImage
     * Used to sort a vector of pointers to Images.
     * @param a
     * @param b
     * @return
     */
    static bool comparePtrToImage(std::shared_ptr<Image> a, std::shared_ptr<Image> b);

    /**
     * @brief generateAnnotatedImage
     * Function used to create the annotated image showing the analysis results.
     */
    void generateAnnotatedImage();

};

#endif // IMAGE_H
