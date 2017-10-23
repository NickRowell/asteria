#include "coordinateutil.h"

#include "util/mathutil.h"

CoordinateUtil::CoordinateUtil()
{

}

/**
 * @brief Converts celestial Right Ascension and Declination to Azimuth and Elevation.
 *
 * See equations 4-11 to 4-14 in "Fundamentals of Astrodynamics and Applications", fourth edition.
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

    // Shift azimuth to 0:2pi range
    translateToRangeZeroToTwoPi(az);
}

/**
 * @brief Computes and returns the matrix that rotates vectors from the BCRF to the ECEF.
 *
 * @param gmst
 *  The Greenwich Mean Sidereal Time [decimal hours]
 * @return
 *  The orthonormal matrix that rotates vectors from the BCRF to the ECEF.
 */
Eigen::Matrix3d CoordinateUtil::getBcrfToEcefRot(const double &gmst) {

    // Convert GMST to radians; the transformation from BCRF to ECEF is a rotation about the Z axis
    // by this amount.
    double theta = MathUtil::toRadians(gmst * 15.0);
    double sinTheta = std::sin(theta);
    double cosTheta = std::cos(theta);

    Eigen::Matrix3d r_bcrf_ecef;
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
Eigen::Matrix3d CoordinateUtil::getEcefToSezRot(const double &lon, const double &lat) {

    double sinLong = std::sin(lon);
    double cosLong = std::cos(lon);
    double sinLat = std::sin(lat);
    double cosLat = std::cos(lat);

    Eigen::Matrix3d r_ecef_sez;

    r_ecef_sez <<  sinLat * cosLong, sinLat * sinLong, -cosLat,
                           -sinLong,          cosLong,     0.0,
                   cosLat * cosLong, cosLat * sinLong,  sinLat;

    return r_ecef_sez;
}

/**
 * @brief Computes and returns the matrix that rotates vectors from the SEZ to the CAM frame.
 *
 * @param az
 *  The azimuthal angle of the camera Z axis (i.e. boresight / pointing direction, measured east-of-north) [radians]
 * @param el
 *  The elevation angle of the camera Z axis (i.e. boresight / pointing direction, measured from horizon) [radians]
 * @param roll
 *  The roll angle about the camera Z axis (positive in the sense of the right-hand-screw rule) [radians]
 * @return
 *  The orthonormal matrix that rotates vectors from the SEZ to the CAM frame.
 */
Eigen::Matrix3d CoordinateUtil::getSezToCamRot(const double &az, const double &el, const double &roll) {

    // Convert azimuth to the east-of-south version for use with SEZ frame
    double az_prime = az;
    CoordinateUtil::eastOfNorthToEastOfSouth(az_prime);

    // Compose the azimuth, elevation and roll rotation matrices.

    // Changes in azimuth rotate the camera about the SEZ frame Z axis
    Eigen::Matrix3d r_az;
    double sinAz = std::sin(az_prime);
    double cosAz = std::cos(az_prime);
    r_az << cosAz, sinAz, 0.0,
           -sinAz, cosAz, 0.0,
              0.0,   0.0, 1.0;

    // Changes in elevation rotate the camera about the SEZ frame Y (E) axis
    Eigen::Matrix3d r_el;
    double sinEl = std::sin(el);
    double cosEl = std::cos(el);
    r_el << cosEl, 0.0,  sinEl,
              0.0, 1.0,    0.0,
           -sinEl, 0.0,  cosEl;

    // Changes in roll rotate the camera about the SEZ frame X (S) axis
    Eigen::Matrix3d r_roll;
    double sinRoll = std::sin(roll);
    double cosRoll = std::cos(roll);
    r_roll << 1.0,     0.0,      0.0,
              0.0, cosRoll, -sinRoll,
              0.0, sinRoll,  cosRoll;

    // Compose the combined rotation by appropriate multiplication of the individual rotations
    Eigen::Matrix3d r_sez_cam = r_roll * r_el * r_az;

    // We need to apply a fixed rotation that aligns the camera and SEZ frame when the
    // azimuth, elevation and roll are all zero.
    Eigen::Matrix3d delta;
    delta << 0, -1,  0,
             0,  0, -1,
             1,  0,  0;

    return delta * r_sez_cam;
}

/**
 * @brief Retrieves the azimuth, elevation and roll angles for the camera frame from the corresponding
 * orthonormal rotation matrix.
 * @param r_sez_cam
 *  The orthonormal matrix that rotates vectors from the SEZ to the CAM frame.
 * @param az
 *  On exit, contains the azimuthal angle of the camera Z axis (i.e. boresight / pointing direction, measured east-of-north) [radians]
 * @param el
 *  On exit, contains the elevation angle of the camera Z axis (i.e. boresight / pointing direction, measured from horizon) [radians]
 * @param roll
 *  On exit, contains the roll angle about the camera Z axis (positive in the sense of the right-hand-screw rule) [radians]
 */
void CoordinateUtil::getAzElRoll(const Eigen::Matrix3d &r_sez_cam, double &az, double &el, double &roll) {

    Eigen::Vector3d r_sez_cam_x = r_sez_cam.row(0);
    Eigen::Vector3d r_sez_cam_y = r_sez_cam.row(1);
    Eigen::Vector3d r_sez_cam_z = r_sez_cam.row(2);

    el = std::asin(r_sez_cam_z[2]);

    az = std::atan2(r_sez_cam_z[1], r_sez_cam_z[0]);
    CoordinateUtil::eastOfSouthToEastOfNorth(az);

    roll = std::atan2(r_sez_cam_x[2], -r_sez_cam_y[2]);
}

/**
 * @brief Composes the camera frame intrinsic matrix from the focal length and sensor size, adopting the
 * pinhole camera model. This is an appriximate method that assumes the principal point (projection of the
 * camera frame origin) is at the centre of the image.
 *
 * @param f
 *  The lens focal length [mm]
 * @param sx
 *  The pixel horiontal width [um]
 * @param sy
 *  The pixel vertical height [um]
 * @param width
 *  The image width [pixels]
 * @param height
 *  The image height [pixels]
 * @return
 *  The corresponding pinhole camera matrix.
 */
Eigen::Matrix3d CoordinateUtil::getCamIntrinsicMatrix(const double &f, const double &sx, const double &sy,
                                               const unsigned int &width, const unsigned int &height) {
    // Horizontal focal length [pixels]
    double fx = 1000.0 * f / sx;
    // Vertical focal length [pixels]
    double fy = 1000.0 * f / sy;

    // Principal point location
    double px = (double)width / 2.0;
    double py = (double)height / 2.0;

    Eigen::Matrix3d r_cam_im;

    r_cam_im <<  fx, 0.0,  px,
                0.0,  fy,  py,
                0.0, 0.0, 1.0;

    return r_cam_im;
}

/**
 * @brief Composes the inverse of the camera frame intrinsic matrix from the focal length and sensor size, adopting the
 * pinhole camera model. This is an appriximate method that assumes the principal point (projection of the
 * camera frame origin) is at the centre of the image.
 *
 * @param f
 *  The lens focal length [mm]
 * @param sx
 *  The pixel horiontal width [um]
 * @param sy
 *  The pixel vertical height [um]
 * @param width
 *  The image width [pixels]
 * @param height
 *  The image height [pixels]
 * @return
 *  The inverse of the corresponding pinhole camera matrix.
 */
Eigen::Matrix3d CoordinateUtil::getCamIntrinsicMatrixInverse(const double &f, const double &sx, const double &sy,
                                               const unsigned int &width, const unsigned int &height) {
    // Horizontal focal length [pixels]
    double fx = 1000.0 * f / sx;
    // Vertical focal length [pixels]
    double fy = 1000.0 * f / sy;

    // Principal point location
    double px = (double)width / 2.0;
    double py = (double)height / 2.0;

    Eigen::Matrix3d r_cam_im;

    r_cam_im <<  1.0/fx,     0.0, -px/fx,
                    0.0,  1.0/fy, -py/fy,
                    0.0,     0.0,    1.0;

    return r_cam_im;
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
void CoordinateUtil::cartesianToSpherical(const Eigen::Vector3d &cart, double &r, double &theta, double &phi) {

    double x = cart[0];
    double y = cart[1];
    double z = cart[2];

    // Length of vector
    r = std::sqrt(x*x + y*y + z*z);

    // declination/latitude/elevation...
    phi = std::asin(z/r);

    // right ascension/longitude/azimuth...
    theta = std::atan2(y, x);

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
void CoordinateUtil::sphericalToCartesian(Eigen::Vector3d &cart, const double &r, const double &theta, const double &phi) {
    cart[0] = r * std::cos(theta) * std::cos(phi);
    cart[1] = r * std::sin(theta) * std::cos(phi);
    cart[2] = r * std::sin(phi);
}

/**
 * @brief Shifts an angle to the equivalent angle in the 0->2*PI range by adding or subtracting multiples
 * of 2*PI as necessary.
 *
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

/**
 * @brief Transforms an angle measured east-of-south to the equivalent angle measured
 * east-of-north. This is useful for converting the equatorial angular coordinate of a
 * position in the SEZ frame to the conventional azimuthal angle.
 *
 * @param angle
 *  The input angle, measured east-of-south. On exit, this will contain the east-of-north angle [radians]
 */
void CoordinateUtil::eastOfSouthToEastOfNorth(double &angle) {
    angle = M_PI - angle;
    CoordinateUtil::translateToRangeZeroToTwoPi(angle);
}

/**
 * @brief Transforms an angle measured east-of-north to the equivalent angle measured
 * east-of-south. This is useful for converting the conventional azimuthal angle to the
 * equatorial angular coordinate of a position in the SEZ frame.
 *
 * @param angle
 *  The input angle, measured east-of-north. On exit, this will contain the east-of-south angle [radians]
 */
void CoordinateUtil::eastOfNorthToEastOfSouth(double &angle) {
    angle = M_PI - angle;
    CoordinateUtil::translateToRangeZeroToTwoPi(angle);
}

/**
 * @brief Computes the partial derivatives of the camera-frame position vector for a point specified in the
 * SEZ frame, with respect to the SEZ-CAM rotation as parameterised by the quaternion elements. This has
 * applications in solving for the camera orientation using reference stars.
 *
 * @param r_sez
 *  The SEZ-frame position vector of the point.
 * @param r_sez_cam
 *  The orthonormal matrix that rotates vectors from the SEZ to the CAM frame.
 * @param dr_cam_dq0
 *  On exit, contains the partial derivatives of the camera-frame position vector elements with respect to
 * the q0 parameter of the camera orientation.
 * @param dr_cam_dq1
 *  On exit, contains the partial derivatives of the camera-frame position vector elements with respect to
 * the q1 parameter of the camera orientation.
 * @param dr_cam_dq2
 *  On exit, contains the partial derivatives of the camera-frame position vector elements with respect to
 * the q2 parameter of the camera orientation.
 * @param dr_cam_dq3
 *  On exit, contains the partial derivatives of the camera-frame position vector elements with respect to
 * the q3 parameter of the camera orientation.
 */
void CoordinateUtil::getSezToCamPartials(const Eigen::Vector3d &r_sez, const Eigen::Matrix3d &r_sez_cam, Eigen::Vector3d &dr_cam_dq0,
                                    Eigen::Vector3d &dr_cam_dq1, Eigen::Vector3d &dr_cam_dq2, Eigen::Vector3d &dr_cam_dq3) {

    double x_sez = r_sez[0];
    double y_sez = r_sez[1];
    double z_sez = r_sez[2];

    Eigen::Quaterniond q(r_sez_cam);
    // Ensure the quaternion is normalised
    q.normalize();

    double q0 = q.w();
    double q1 = q.x();
    double q2 = q.y();
    double q3 = q.z();

    dr_cam_dq0[0] =   2.0 * q0 * x_sez - 2.0 * q3 * y_sez + 2.0 * q2 * z_sez;
    dr_cam_dq0[1] =   2.0 * q3 * x_sez + 2.0 * q0 * y_sez - 2.0 * q1 * z_sez;
    dr_cam_dq0[2] = - 2.0 * q2 * x_sez + 2.0 * q1 * y_sez + 2.0 * q0 * z_sez;

    dr_cam_dq1[0] =   2.0 * q1 * x_sez + 2.0 * q2 * y_sez + 2.0 * q3 * z_sez;
    dr_cam_dq1[1] =   2.0 * q2 * x_sez - 2.0 * q1 * y_sez - 2.0 * q0 * z_sez;
    dr_cam_dq1[2] =   2.0 * q3 * x_sez + 2.0 * q0 * y_sez - 2.0 * q1 * z_sez;

    dr_cam_dq2[0] = - 2.0 * q2 * x_sez + 2.0 * q1 * y_sez + 2.0 * q0 * z_sez;
    dr_cam_dq2[1] =   2.0 * q1 * x_sez + 2.0 * q2 * y_sez + 2.0 * q3 * z_sez;
    dr_cam_dq2[2] = - 2.0 * q0 * x_sez + 2.0 * q3 * y_sez - 2.0 * q2 * z_sez;

    dr_cam_dq3[0] = - 2.0 * q3 * x_sez - 2.0 * q0 * y_sez + 2.0 * q1 * z_sez;
    dr_cam_dq3[1] =   2.0 * q0 * x_sez - 2.0 * q3 * y_sez + 2.0 * q2 * z_sez;
    dr_cam_dq3[2] =   2.0 * q1 * x_sez + 2.0 * q2 * y_sez + 2.0 * q3 * z_sez;
}
