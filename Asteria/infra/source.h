#ifndef SOURCE_H
#define SOURCE_H

#include <vector>

/**
 * @brief Represents a source extracted from an image.
 */
class Source
{
public:

    /**
     * @brief Default constructor for the Source.
     */
    Source();

    /**
     * @brief Copy constructor for the Source.
     * @param copyme
     *  The Source to copy
     */
    Source(const Source& copyme);

    /**
     * @brief Copy-assignment operator for the Source.
     * @param copyme
     *  The Source to copy
     * @return
     *  A reference to this Source
     */
    Source& operator=(const Source& copyme);

    /**
     * @brief Contains the indices of the pixels assigned to this source.
     */
    std::vector<unsigned int> pixels;

    /**
     * @brief Integrated, background-subtracted samples [ADU]
     */
    double adu;

    /**
     * @brief Uncertainty (one standard deviation) on the integrated, background-subtracted samples [ADU]
     */
    double sigma_adu;

    /**
     * @brief Flux-weighted centroid for the source samples [pixels]
     */
    double i, j;

    /**
     * @brief Flux-weighted dispersion matrix for the source samples [pixels^2]
     */
    double c_ii, c_ij, c_jj;

    /**
     * @brief Eigenvalues of the flux-weighted sample dispersion matrix [pixels^2]
     */
    double l1, l2;

    /**
     * @brief Orientation of the flux-weighted sample dispersion matrix [radians]. This is the angle
     * between the major axis of the dispersion matrix and the X axis of the image.
     */
    double orientation;
};

#endif // SOURCE_H
