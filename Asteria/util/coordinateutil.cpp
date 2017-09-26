#include "coordinateutil.h"

#include "util/mathutil.h"

CoordinateUtil::CoordinateUtil()
{

}

/**
 * @brief Converts celestial Right Ascension and Declination to Azimuth and Elevation.
 *
 * @param ra
 *  The right ascension [radians]
 * @param dec
 *  The declination [radians]
 * @param lat
 *  The latitude of the observing site [radians]
 * @param lst
 *  The Local Sidereal Time [decimal hours]
 * @param az
 *  On exit, contains the azimuth [radians]
 * @param el
 *  On exit, contains the elevation [radians]
 */
void CoordinateUtil::raDecToAzEl(const double &ra, const double &dec, const double &lat, const double &lst, double &az, double &el) {

    double lha = MathUtil::toRadians(lst * 15) - ra;

    double sinEl = std::sin(lat) * std::sin(dec) + std::cos(lat) * std::cos(dec) * std::cos(lha);
    double cosEl = std::sqrt(1 - sinEl*sinEl);

    double sinAz = - (std::sin(lha) * std::cos(dec) / cosEl);
    double cosAz = (std::sin(dec) - sinEl * std::sin(lat)) / (cosEl * std::cos(lat));

    el = std::asin(sinEl);
    az = std::atan2(sinAz, cosAz);
}

/**
 * @brief Computes and returns the matrix that rotates vectors from the BCRF to the ECEF.
 *
 * @param gmst
 *  The Greenwich Mean Sidereal Time [decimal hours]
 * @return
 *  The orthonormal matrix that rotates vectors from the BCRF to the ECEF.
 */
Matrix3d CoordinateUtil::getBcrfToEcefRot(const double &gmst) {

    // Convert GMST to radians; the transformation from BCRF to ECEF is a rotation about the Z axis
    // by this amount.
    double theta = MathUtil::toRadians(gmst * 15.0);
    double sinTheta = std::sin(theta);
    double cosTheta = std::cos(theta);

    Matrix3d r_bcrf_ecef;
    r_bcrf_ecef <<  cosTheta, sinTheta, 0.0,
                   -sinTheta, cosTheta, 0.0,
                         0.0,      0.0, 1.0;

    return r_bcrf_ecef;
}

/**
 * @brief Computes and returns the matrix that rotates vectors from the ECEF to the SEZ frame.
 *
 * See algorithm 51 (SITE-TRACK) in "Fundamentals of Astrodynamics and Applications", fourth edition.
 *
 * @param lon
 *  The longitude of the observing site [radians]
 * @param lat
 *  The latitude of the observing site [radians]
 * @return
 *  The orthonormal matrix that rotates vectors from the ECEF to the SEZ frame.
 */
Matrix3d CoordinateUtil::getEcefToSezRot(const double &lon, const double &lat) {

    double sinLong = std::sin(lon);
    double cosLong = std::cos(lon);
    double sinLat = std::sin(lat);
    double cosLat = std::cos(lat);

    Matrix3d r_ecef_sez;

    r_ecef_sez <<  sinLat * cosLong, sinLat * sinLong, -cosLat,
                           -sinLong,          cosLong,     0.0,
                   cosLat * cosLong, cosLat * sinLong, sinLong;

    return r_ecef_sez;
}

/**
 * @brief Converts a position vector from the Solar System Barycentric Reference Frame to the Earth-Centred
 * Earth-Fixed Frame.
 *
 * Note that only a rotation is applied, no translation; therefore this method is only applicable to directions
 * towards distant reference stars.
 *
 * @param r_bcrf
 *  The position vector in the BCRF frame, Cartesian coordinates [arbitrary units]
 * @param r_ecef
 *  On exit, contains the position vector in the ECEF frame, Cartesian coordinates [arbitrary units]
 * @param lst
 *  The Greenwich Mean Sideral Time [decimal hours]
 */
void bcrfToEcef(const Vector3d &r_bcrf, Vector3d &r_ecef, const double &gmst) {

}

/**
 * @brief Converts position vector from Cartesian coordinates to spherical polar coordinates.
 *
 * @param cart
 *  The position vector in Cartesian coordinates [arbitrary units]
 * @param r
 *  On exit, contains the radial coordinate [same units as input]
 * @param theta
 *  On exit, contains the angular coordinate around the equatorial plane, positive east (right ascension/longitude/azimuth) [radians]
 * @param phi
 *  On exit, contains the angular coordinate perpendicular to the equatorial plane, positive north (declination/latitude/elevation) [radians]
 */
void CoordinateUtil::cartesianToSpherical(const Vector3d &cart, double &r, double &theta, double &phi) {

    double x = cart[0];
    double y = cart[1];
    double z = cart[2];

    // Length of vector
    r = std::sqrt(x*x + y*y + z*z);

    // declination/latitude/elevation...
    phi = std::asin(z/r);

    // right ascension/longitude/azimuth...
    theta  = std::atan2(y, x);

    // Shift theta to 0:2pi range
    translateToRangeZeroToTwoPi(theta);
}

/**
 * @brief Converts position vector from spherical polar coordinates to Cartesian coordinates.
 *
 * @param cart
 *  On exit, contains the position vector in Cartesian coordinates [same units as input]
 * @param r
 *  The radial coordinate [arbitrary units]
 * @param theta
 *  The angular coordinate around the equatorial plane, positive east (right ascension/longitude/azimuth) [radians]
 * @param phi
 *  The angular coordinate perpendicular to the equatorial plane, positive north (declination/latitude/elevation) [radians]
 */
void CoordinateUtil::sphericalToCartesian(Vector3d &cart, const double &r, const double &theta, const double &phi) {
    cart[0] = r * std::cos(theta) * std::cos(phi);
    cart[1] = r * std::sin(theta) * std::cos(phi);
    cart[2] = r * std::sin(phi);
}

/**
 * @brief Shifts an angle to the equivalent angle in the 0->2*PI range by adding or subtracting multiples
 * of 2*PI as necessary.
 * @param angle
 *  The angle; on exit this is within the range 0->2*PI [radians]
 */
void CoordinateUtil::translateToRangeZeroToTwoPi(double &angle) {
    if(angle < 0.0) {
        // Add multiples of 2*pi
        double multiplesOfTwoPiToAdd = std::floor(-angle/(2.0*M_PI)) + 1.0;
        angle += multiplesOfTwoPiToAdd * 2.0 * M_PI;
    }
    else if(angle > 2.0*M_PI) {
        // Subtract multiples of 2*pi
        double multiplesOfTwoPiToSubtract = std::floor(angle/(2.0*M_PI));
        angle -= multiplesOfTwoPiToSubtract * 2.0 * M_PI;
    }
}
