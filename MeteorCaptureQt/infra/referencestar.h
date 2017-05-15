#ifndef REFERENCESTAR_H
#define REFERENCESTAR_H

#include <string>
#include <vector>
#include "math/vec3.h"

class ReferenceStar
{
public:
    ReferenceStar();
    ReferenceStar(const ReferenceStar& copyme);
    ReferenceStar(double ra, double dec, double mag);

    /**
     * @brief loadCatalogue Utility function to load the reference star catalogue file.
     * @param path
     * @return
     */
    static std::vector<ReferenceStar> loadCatalogue(std::string &path);

    /**
     * @brief ra
     * The (fixed) Right Ascension of the reference star [radians]
     */
    const double ra;

    /**
     * @brief dec
     * The (fixed) Declination of the reference star [radians]
     */
    const double dec;

    /**
     * @brief vt
     * The (fixed) apparent magnitude of the reference star [mag]
     */
    const double mag;

    /**
     * @brief r
     * Camera-frame unit vector pointing towards the star given the current camera orientation and
     * time.
     */
    Vec3<double> r;




};

#endif // REFERENCESTAR_H
