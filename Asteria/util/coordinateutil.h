#ifndef COORDINATEUTIL_H
#define COORDINATEUTIL_H

// Eigen is used to provide vector algebra
#include <Eigen/Dense>

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

    static Eigen::Matrix3d getBcrfToEcefRot(const double &gmst);

    static Eigen::Matrix3d getEcefToSezRot(const double &lon, const double &lat);

    static Eigen::Matrix3d getSezToCamRot(const double &az, const double &el, const double &roll);

    static void getAzElRoll(const Eigen::Matrix3d &r_sez_cam, double &az, double &el, double &roll);

    static Eigen::Matrix3d getCamIntrinsicMatrix(const double &f, const double &sx, const double &sy, const unsigned int &width, const unsigned int &height);

    static Eigen::Matrix3d getCamIntrinsicMatrixInverse(const double &f, const double &sx, const double &sy, const unsigned int &width, const unsigned int &height);

    static void cartesianToSpherical(const Eigen::Vector3d &cart, double &r, double &theta, double &phi);

    static void sphericalToCartesian(Eigen::Vector3d &cart, const double &r, const double &theta, const double &phi);

    static void translateToRangeZeroToTwoPi(double &angle);

    static void eastOfSouthToEastOfNorth(double &angle);

    static void eastOfNorthToEastOfSouth(double &angle);

    static void getSezToCamPartials(const Eigen::Vector3d &r_sez, const Eigen::Matrix3d &r_sez_cam, Eigen::Vector3d &dr_cam_dq0,
                                    Eigen::Vector3d &dr_cam_dq1, Eigen::Vector3d &dr_cam_dq2, Eigen::Vector3d &dr_cam_dq3);
};

#endif // COORDINATEUTIL_H
