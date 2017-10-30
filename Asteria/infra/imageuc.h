#ifndef IMAGE_H
#define IMAGE_H

#include "infra/image.h"
#include "infra/meteorimagelocationmeasurement.h"

#include <iostream>
#include <linux/videodev2.h>

class Imageuc : public Image<unsigned char>
{

public:

    Imageuc();
    Imageuc(const Imageuc& copyme);
    Imageuc(unsigned int &width, unsigned int &height);
    Imageuc(unsigned int &width, unsigned int &height, unsigned char val);
    ~Imageuc();

    /**
     * @brief The value of the v4l2_field enum representing the field order of the image.
     * Currently only the following types are supported:
     * V4L2_FIELD_NONE (progressive format; not interlaced)
     * V4L2_FIELD_INTERLACED (interleaved/interlaced format)
     * V4L2_FIELD_INTERLACED_TB (interleaved/interlaced format; top field is transmitted first)
     * V4L2_FIELD_INTERLACED_BT (interleaved/interlaced format; bottom field is transmitted first)
     */
    unsigned int field;

    // Optional RGBA overlay image with annotations, for display.
    // Not to be computed if it's not being displayed in real time.
    std::vector<unsigned int> annotatedImage;

    void writeToStream(std::ostream &output) const;
    void readFromStream(std::istream &input);

    /**
     * @brief Function used to create the annotated image showing the analysis results for the current frame.
     */
    void generateAnnotatedImage(const MeteorImageLocationMeasurement &loc);

    /**
     * @brief Function used to create the annotated image for the peakHold image showing the analysis
     * results for the entire clip.
     *
     * @param eventFrames
     */
    void generatePeakholdAnnotatedImage(std::vector<std::shared_ptr<Imageuc> > &eventFrames, const std::vector<MeteorImageLocationMeasurement> &locs);

};

#endif // IMAGE_H
