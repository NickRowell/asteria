#ifndef SAMPLE_H
#define SAMPLE_H

/**
 * @brief The Sample class
 * Represents a single sample in an image. Enacpsulates both the pixel coordinate(s) and the level.
 * This is very useful e.g. in the source detection algorithm.
 */
class Sample
{
public:

    Sample(unsigned int index, unsigned int width, unsigned char level);
    Sample(const Sample& copyme);

    /**
     * @brief index
     * Sample index, i.e. the order of the pixel in the image in row-major order.
     */
    unsigned int index;
    /**
     * @brief label
     * Used during source extraction to label a sample as being part of a source. A value zero
     * means 'unlabelled', i.e. not part of a source, and any other value e.g. 1 means 'part of
     * source 1'.
     */
    unsigned int label;
    /**
     * @brief i
     * The i-coordinate of the sample, i.e. the position in the left-right direction.
     * This can be determined from the index and the image width by:
     *
     *  i = index % width;
     *
     */
    unsigned int i;
    /**
     * @brief j
     * The j-coordinate of the sample, i.e. the position in the up-down direction.
     * This can be determined from the index and the image width by:
     *
     *  i = index / width;
     */
    unsigned int j;
    /**
     * @brief level
     * Sample level [0:255]
     */
    unsigned char level;

    /**
     * Function used to aid sorting of a vector of Samples into ascending order of level.
     *
     * @brief operator <
     * @param sample
     * @return
     */
    bool operator < (const Sample& sample) const;

    bool operator==(const Sample& rhs) const;

    /**
     * @brief compareSampleDecreasing
     * Used to sort a vector of Samples into increasing order of level.
     * @param a
     *  The first sample
     * @param b
     *  The second sample
     * @return
     */
    static bool compareSampleIncreasing(const Sample &a, const Sample &b);
    static bool compareSampleDecreasing(const Sample &a, const Sample &b);
    static bool compareSamplePtrIncreasing(const Sample *a, const Sample *b);
    static bool compareSamplePtrDecreasing(const Sample *a, const Sample *b);
};

#endif // SAMPLE_H
