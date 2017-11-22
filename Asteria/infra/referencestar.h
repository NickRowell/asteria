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
    ReferenceStar& operator=(const ReferenceStar& copyme);

    /**
     * @brief Utility function to load the reference star catalogue file.
     * @param
     *  The path to the reference star catalogue file.
     * @return
     *  A vector of ReferenceStars loaded from the file.
     */
    static std::vector<ReferenceStar> loadCatalogue(std::string &path);

    /**
     * @brief The Right Ascension of the reference star [radians]
     */
    double ra;

    /**
     * @brief The Declination of the reference star [radians]
     */
    double dec;

    /**
     * @brief The apparent magnitude of the reference star [mag]
     */
    double mag;

    /**
     * @brief The current image plane i coordinate of the reference star [pixels]
     */
    double i;

    /**
     * @brief The current image plane j coordinate of the reference star [pixels]
     */
    double j;

    /**
     * @brief Camera-frame unit vector pointing towards the star given the current camera orientation and
     * time.
     */
    Vector3d r;

};

#endif // REFERENCESTAR_H
