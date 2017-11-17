#ifndef SAMPLE_H
#define SAMPLE_H

/**
 * @brief The Sample class
 * Represents a single sample in an image. Enacpsulates both the pixel coordinate(s) and the level.
 * This is very useful e.g. in the source detection algorithm.
 */
template<class T>
class Sample
{
public:

    Sample(unsigned int index, unsigned int width, unsigned char level) : index(index), label(0), i(index % width), j(index / width), level(level) {

    }

    Sample(const Sample& copyme) : index(copyme.index), label(copyme.label), i(copyme.i), j(copyme.j), level(copyme.level) {

    }

    /**
     * @brief Sample index, i.e. the order of the pixel in the image in row-major order.
     */
    unsigned int index;

    /**
     * @brief Used during source extraction to label a sample as being part of a source. A value zero
     * means 'unlabelled', i.e. not part of a source, and any other value e.g. 1 means 'part of
     * source 1'.
     */
    unsigned int label;

    /**
     * @brief The i-coordinate of the sample, i.e. the position in the horizontal direction.
     * This can be determined from the index and the image width by:
     *
     *  i = index % width;
     *
     */
    unsigned int i;

    /**
     * @brief  The j-coordinate of the sample, i.e. the position in the vertical direction.
     * This can be determined from the index and the image width by:
     *
     *  i = index / width;
     */
    unsigned int j;

    /**
     * @brief Sample level [0:255]
     */
    T level;

    /**
     * @brief Function used to aid sorting of a vector of Samples into ascending order of level.
     * @param sample
     *  The Sample to compare with this one.
     * @return
     *  True if this Sample is less than the given one, false otherwise.
     */
    bool operator < (const Sample& sample) const {
        return (level < sample.level);
    }

    /**
     * @brief Equality-equals operation; tests if two Samples are the same Sample.
     * @param rhs
     *  The Sample to compare with this one.
     * @return
     *  True if this Sample is equal to the given one, false otherwise. Samples are compared on the
     * pixel index alone, i.e. two are considered equal if they correspond to the same pixel, although
     * they may have different values.
     */
    bool operator==(const Sample& sample) const {
        if (this->index==sample.index) {
            return true;
        }
        return false;
    }

    /**
     * @brief Used to sort a vector of Samples into increasing order of level.
     * @param a
     *  The first sample
     * @param b
     *  The second sample
     * @return
     */
    static bool compareSampleIncreasing(const Sample &a, const Sample &b) {
        return (a < b);
    }

    static bool compareSampleDecreasing(const Sample &a, const Sample &b) {
        return (b < a);
    }

    static bool compareSamplePtrIncreasing(const Sample *a, const Sample *b) {
        return (*a < *b);
    }

    static bool compareSamplePtrDecreasing(const Sample *a, const Sample *b) {
        return (*b < *a);
    }
};

#endif // SAMPLE_H
