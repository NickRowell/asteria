#include "coordinateutil.h"

#include "util/mathutil.h"

CoordinateUtil::CoordinateUtil()
{

}

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

void CoordinateUtil::getAzElRoll(const Eigen::Matrix3d &r_sez_cam, double &az, double &el, double &roll) {

    Eigen::Vector3d r_sez_cam_x = r_sez_cam.row(0);
    Eigen::Vector3d r_sez_cam_y = r_sez_cam.row(1);
    Eigen::Vector3d r_sez_cam_z = r_sez_cam.row(2);

    el = std::asin(r_sez_cam_z[2]);

    az = std::atan2(r_sez_cam_z[1], r_sez_cam_z[0]);
    CoordinateUtil::eastOfSouthToEastOfNorth(az);

    roll = std::atan2(r_sez_cam_x[2], -r_sez_cam_y[2]);
}

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

void CoordinateUtil::sphericalToCartesian(Eigen::Vector3d &cart, const double &r, const double &theta, const double &phi) {
    cart[0] = r * std::cos(theta) * std::cos(phi);
    cart[1] = r * std::sin(theta) * std::cos(phi);
    cart[2] = r * std::sin(phi);
}

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

void CoordinateUtil::eastOfSouthToEastOfNorth(double &angle) {
    angle = M_PI - angle;
    CoordinateUtil::translateToRangeZeroToTwoPi(angle);
}

void CoordinateUtil::eastOfNorthToEastOfSouth(double &angle) {
    angle = M_PI - angle;
    CoordinateUtil::translateToRangeZeroToTwoPi(angle);
}

void CoordinateUtil::getSezToCamPartials(const Eigen::Vector3d &r_sez, const Quaterniond &q_sez_cam, Eigen::Vector3d &dr_cam_dq0,
                                    Eigen::Vector3d &dr_cam_dq1, Eigen::Vector3d &dr_cam_dq2, Eigen::Vector3d &dr_cam_dq3) {

    double x_sez = r_sez[0];
    double y_sez = r_sez[1];
    double z_sez = r_sez[2];

    double q0 = q_sez_cam.w();
    double q1 = q_sez_cam.x();
    double q2 = q_sez_cam.y();
    double q3 = q_sez_cam.z();

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

void CoordinateUtil::projectReferenceStar(ReferenceStar &star, const Eigen::Quaterniond &q_sez_cam, const CameraModelBase &cam, const double &gmst,
                                          const double &lon, const double &lat){

    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);

    // Convert q_sez_cam to rotation matrix as it's much more efficient for transforming many vectors
    Matrix3d r_sez_cam = q_sez_cam.toRotationMatrix();

    // Full transformation BCRF->CAM
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    CoordinateUtil::projectReferenceStar(star, r_bcrf_cam, cam);
}

void CoordinateUtil::projectReferenceStar(ReferenceStar &star, const Eigen::Matrix3d &r_bcrf_cam, const CameraModelBase &cam) {

    // Unit vector towards star in BCRF frame:
    Vector3d r_bcrf;
    CoordinateUtil::sphericalToCartesian(r_bcrf, 1.0, star.ra, star.dec);

    // Transform to CAM frame:
    star.r = r_bcrf_cam * r_bcrf;

    // Project into image coordinates
    star.visible = cam.projectVector(star.r, star.i, star.j);
}
