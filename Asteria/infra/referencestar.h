#ifndef REFERENCESTAR_H
#define REFERENCESTAR_H

#include <string>
#include <vector>

#include <Eigen/Dense>

using namespace Eigen;

class ReferenceStar
{
public:

    /**
     * @brief Default constructor for the ReferenceStar.
     */
    ReferenceStar();

    /**
     * @brief Copy constructor for the ReferenceStar.
     * @param copyme
     *  The ReferenceStar to copy
     */
    ReferenceStar(const ReferenceStar& copyme);

    /**
     * @brief Main constructor for the ReferenceStar.
     * @param ra
     *  The right ascension [radians]
     * @param dec
     *  The declination [radians]
     * @param mag
     *  The magnitude [mags]
     */
    ReferenceStar(const double &ra, const double &dec, const double &mag);

    /**
     * @brief Copy-assignment operator.
     * @param copyme
     *  The ReferenceStar to copy
     * @return
     *  A reference to this ReferenceStar
     */
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

    /**
     * @brief Boolean flag indicating if the star is visible in the current image.
     */
    bool visible;

};

#endif // REFERENCESTAR_H
