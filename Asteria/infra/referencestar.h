#ifndef REFERENCESTAR_H
#define REFERENCESTAR_H

#include <string>
#include <vector>

#include <Eigen/Dense>

using namespace Eigen;

class ReferenceStar
{
public:
    ReferenceStar();
    ReferenceStar(const ReferenceStar& copyme);
    ReferenceStar(const double &ra, const double &dec, const double &mag);

    /**
     * @brief Utility function to load the reference star catalogue file.
     * @param
     *  The path to the reference star catalogue file.
     * @return
     *  A vector of ReferenceStars loaded from the file.
     */
    static std::vector<ReferenceStar> loadCatalogue(std::string &path);

    /**
     * @brief The (fixed) Right Ascension of the reference star [radians]
     */
    const double ra;

    /**
     * @brief The (fixed) Declination of the reference star [radians]
     */
    const double dec;

    /**
     * @brief The (fixed) apparent magnitude of the reference star [mag]
     */
    const double mag;

    /**
     * @brief Camera-frame unit vector pointing towards the star given the current camera orientation and
     * time.
     */
    Vector3d r;

};

#endif // REFERENCESTAR_H
