#ifndef SOURCE_H
#define SOURCE_H

#include <vector>

/**
 * @brief The Source class
 * Represents a source extracted from an image.
 */
class Source
{
public:
    Source();
//    Source(const Source& copyme);

    /**
     * @brief pixels
     * Contains the indices of the pixels assigned to this source
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
    double x0, y0;

    /**
     * @brief Flux-weighted dispersion matrix for the source samples [pixels^2]
     */
    double c_xx, c_xy, c_yy;

    /**
     * @brief Eigenvalues of the flux-weighted sample dispersion matrix [pixels^2]
     */
    double l1, l2;

    /**
     * @brief Orientation of the flux-weighted sample dispersion matrix [rads]. This is the angle
     * between the major axis of the dispersion matrix and the X axis of the image.
     */
    double orientation;
};

#endif // SOURCE_H
