#ifndef COORDINATEUTIL_H
#define COORDINATEUTIL_H

#include "infra/referencestar.h"
#include "optics/cameramodelbase.h"

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
    static void raDecToAzEl(const double &ra, const double &dec, const double &lat, const double &lst, double &az, double &el);

    /**
     * @brief Computes and returns the matrix that rotates vectors from the BCRF to the ECEF.
     *
     * @param gmst
     *  The Greenwich Mean Sidereal Time [decimal hours]
     * @return
     *  The orthonormal matrix that rotates vectors from the BCRF to the ECEF.
     */
    static Eigen::Matrix3d getBcrfToEcefRot(const double &gmst);

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
    static Eigen::Matrix3d getEcefToSezRot(const double &lon, const double &lat);

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
    static Eigen::Matrix3d getSezToCamRot(const double &az, const double &el, const double &roll);

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
    static void getAzElRoll(const Eigen::Matrix3d &r_sez_cam, double &az, double &el, double &roll);

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
    static Eigen::Matrix3d getCamIntrinsicMatrix(const double &f, const double &sx, const double &sy, const unsigned int &width, const unsigned int &height);

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
    static Eigen::Matrix3d getCamIntrinsicMatrixInverse(const double &f, const double &sx, const double &sy, const unsigned int &width, const unsigned int &height);

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
    static void cartesianToSpherical(const Eigen::Vector3d &cart, double &r, double &theta, double &phi);

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
    static void sphericalToCartesian(Eigen::Vector3d &cart, const double &r, const double &theta, const double &phi);

    /**
     * @brief Shifts an angle to the equivalent angle in the 0->2*PI range by adding or subtracting multiples
     * of 2*PI as necessary.
     *
     * @param angle
     *  The angle; on exit this is within the range 0->2*PI [radians]
     */
    static void translateToRangeZeroToTwoPi(double &angle);

    /**
     * @brief Transforms an angle measured east-of-south to the equivalent angle measured
     * east-of-north. This is useful for converting the equatorial angular coordinate of a
     * position in the SEZ frame to the conventional azimuthal angle.
     *
     * @param angle
     *  The input angle, measured east-of-south. On exit, this will contain the east-of-north angle [radians]
     */
    static void eastOfSouthToEastOfNorth(double &angle);

    /**
     * @brief Transforms an angle measured east-of-north to the equivalent angle measured
     * east-of-south. This is useful for converting the conventional azimuthal angle to the
     * equatorial angular coordinate of a position in the SEZ frame.
     *
     * @param angle
     *  The input angle, measured east-of-north. On exit, this will contain the east-of-south angle [radians]
     */
    static void eastOfNorthToEastOfSouth(double &angle);

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
    static void getSezToCamPartials(const Eigen::Vector3d &r_sez, const Eigen::Matrix3d &r_sez_cam, Eigen::Vector3d &dr_cam_dq0,
                                    Eigen::Vector3d &dr_cam_dq1, Eigen::Vector3d &dr_cam_dq2, Eigen::Vector3d &dr_cam_dq3);

    /**
     * @brief Computes the camera frame position vector and projected image plane coordinates of the reference star. The
     * star may be behind the camera, in which case the z component of the position vector will be negative and the star
     * will not appear in the image even if the position projects to coordinates within the boundary of the image. Note that
     * this function is inefficient due to the overheads of computing the BCRF->CAM transformation and for projecting multiple
     * ReferenceStars the CoordinateUtil::projectReferenceStar(ReferenceStar &, const Eigen::Matrix3d &, const CameraModelBase &)
     * function should be used instead.
     * @param star
     *  The ReferenceStar to project.
     * @param q_sez_cam
     *  The unit quaternion that rotates vectors from the SEZ to the CAM frame.
     * @param cam
     *  The CameraModelBase that encapsulates the intrinsic parameters of the camera frame
     * @param gmst
     *  The Greenwich Mean Sidereal Time [decimal hours]
     * @param lon
     *  The longitude of the observing site [radians]
     * @param lat
     *  The latitude of the observing site [radians]
     */
    static void projectReferenceStar(ReferenceStar &star, const Eigen::Quaterniond &q_sez_cam, const CameraModelBase &cam, const double &gmst,
                                     const double &lon, const double &lat);

    static void projectReferenceStar(ReferenceStar &star, const Eigen::Matrix3d &r_bcrf_cam, const CameraModelBase &cam);

};

#endif // COORDINATEUTIL_H
