#ifndef COORDINATEUTIL_H
#define COORDINATEUTIL_H

// Eigen is used to provide vector algebra
#include <Eigen/Dense>

using namespace Eigen;

/**
 * @brief The CoordinateUtil class
 * Utilities for coordinate transformation etc.
 */
class CoordinateUtil
{
public:
    CoordinateUtil();

    /**
     * @brief Enumerates the coordinate frames used in this project
     */
    enum CoordinateFrame{
        BCRF,    /*!< Solar System Barycentric Reference Frame; inertial */
        ECEF,    /*!< Earth-Centred Earth-Fixed frame; non-inertial */
        SEZ,     /*!< Local topocentric frame in which azimuth and elevation are defined, formed by South-East-Zenith vectors; non-inertial */
        CAM,     /*!< Camera frame, formed by right-down-forwards vectors relative to image plane; non-inertial */
    };

    static void raDecToAzEl(const double &ra, const double &dec, const double &lat, const double &lst, double &az, double &el);

    static Matrix3d getBcrfToEcefRot(const double &gmst);

    static Matrix3d getEcefToSezRot(const double &lon, const double &lat);

    static Matrix3d getSezToCamRot(const double &az, const double &el, const double &roll);

    static void bcrfToEcef(const Vector3d &r_bcrf, Vector3d &r_ecef, const double &gmst);

    static void cartesianToSpherical(const Vector3d &cart, double &r, double &theta, double &phi);

    static void sphericalToCartesian(Vector3d &cart, const double &r, const double &theta, const double &phi);

    static void translateToRangeZeroToTwoPi(double &angle);

    static void eastOfSouthToEastOfNorth(double &angle);
};

#endif // COORDINATEUTIL_H
