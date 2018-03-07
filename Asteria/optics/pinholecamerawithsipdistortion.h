#ifndef PINHOLECAMERAWITHRADIALANDTANGENTIALDISTORTION_H
#define PINHOLECAMERAWITHRADIALANDTANGENTIALDISTORTION_H

#include "optics/pinholecamera.h"

/**
 * @brief The PinholeCameraWithSipDistortion class provides an implementation of the
 * CameraModelBase for modelling pinhole cameras with general polynomial distortion.
 * This is an implementation of the model described in
 *
 * "The SIP Convention for Representing Distortion in FITS Image Headers"
 *
 * that is a standard adopted by the World Coordinate System.
 */
class PinholeCameraWithSipDistortion : public PinholeCamera {

public:

    /**
     * @brief Default constructor for the PinholeCameraWithSipDistortion.
     */
    PinholeCameraWithSipDistortion();

    /**
     * @brief Main constructor for the PinholeCameraWithSipDistortion.
     *
     * @param width
     *  Width of the detector [pixels]
     * @param height
     *  Height of the detector [pixels]
     * @param fi
     *  Focal length in the i (horizontal) direction [pixels]
     * @param fj
     *  Focal length in the j (vertical) direction [pixels]
     * @param pi
     *  Coordinate of the principal point in the i (horizontal) direction [pixels]
     * @param pj
     *  Coordinate of the principal point in the j (vertical) direction [pixels]
     */
    PinholeCameraWithSipDistortion(const unsigned int &width, const unsigned int &height, const double &fi, const double &fj, const double &pi, const double &pj,
                                   const double &d0, const double &d1, const double &d2, const double &d3, const double &d4, const double &d5, const double &d6,
                                   const double &e0, const double &e1, const double &e2, const double &e3, const double &e4, const double &e5, const double &e6);

    ~PinholeCameraWithSipDistortion();

    /**
     * @brief Coefficients of i' distortion function
     */
    double d0, d1, d2, d3, d4, d5, d6;

    /**
     * @brief Coefficients of j' distortion function
     */
    double e0, e1, e2, e3, e4, e5, e6;

    /**
     * @brief Threshold on the radial distance of (undistorted) points from the distortion centre [pixels].
     * Projected undistorted points that lie at radial distances above this threshold are determined to be not
     * visible in the image even if the distorted point lies within the image area. It is necessary to perform
     * this check because rays far outside the field-of-view can formally project into the image area due to
     * extreme radial distortion corrections way off the optical axis, which should not be applied.
     */
    double r_max;

    PinholeCamera * convertToPinholeCamera() const;

    PinholeCameraWithRadialDistortion * convertToPinholeCameraWithRadialDistortion() const;

    PinholeCameraWithSipDistortion * convertToPinholeCameraWithSipDistortion() const;

    unsigned int getNumParameters() const;

    void getParameters(double *params) const;

    void getIntrinsicPartialDerivatives(double * derivs, const Eigen::Vector3d & r_cam) const;

    void getExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_sez, const Eigen::Quaterniond &q_sez_cam) const;

    void setParameters(const double *);

    Eigen::Vector3d deprojectPixel(const double & i, const double & j) const;

    bool projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const;

    std::string getModelName() const;

    void init();

    /**
     * @brief This function returns the displacement from the ideal (undistorted) pixel coordinate (i, j)
     * of the observed (distorted) pixel coordinate (ip, jp) given the distortion model for the camera.
     *
     * @param i
     *  The ideal (undistorted) pixel coordinate i
     * @param j
     *  The ideal (undistorted) pixel coordinate j
     * @param di
     *  On exit, contains the displacement between the ideal (undistorted) pixel coordinate i and the observed (distorted) pixel coordinate i^prime
     * @param dj
     *  On exit, contains the displacement between the ideal (undistorted) pixel coordinate j and the observed (distorted) pixel coordinate j^prime
     */
    void getForwardDistortionOffset(const double &i, const double &j, double &di, double &dj) const;

    /**
     * @brief This function returns the displacement from the observed (distorted) pixel coordinate (ip, jp)
     * of the ideal (undistorted) pixel coordinate (i, j) given the distortion model for the camera.
     *
     * @param ip
     *  The observed (distorted) pixel coordinate i^prime
     * @param jp
     *  The observed (distorted) pixel coordinate j^prime
     * @param dip
     *  On exit, contains the displacement between the observed (distorted) pixel coordinate i^prime and the ideal (undistorted) pixel coordinate i
     * @param djp
     *  On exit, contains the displacement between the observed (distorted) pixel coordinate j^prime and the ideal (undistorted) pixel coordinate j
     * @param tol
     *  Inversion is iterated until this tolerance is reached, i.e. undistorted location is accurate to within this number of pixels.
     */
    void getInverseDistortionOffset(const double &ip, const double &jp, double &dip, double &djp, const double tol) const;

    /**
     * @brief Get the partial derivatives of the distortion offset in (i,j) with respect to each of
     * the intrinsic parameters of the camera model.
     *
     * @param derivs
     *  Pointer to the start of the array of double values that on exit will contain the partial
     * derivatives of the distortion offset in (i,j) with respect to the intrinsic parameters of the geometric
     * @param r_cam
     *  Position vector of the point in the CAM frame.
     */
    void getForwardDistortionIntrinsicPartialDerivatives(double * derivs, const Eigen::Vector3d & r_cam) const;

    /**
     * @brief Get the partial derivatives of the distortion offset in (i,j) with respect to each of
     * the extrinsic parameters of the camera model, being the four quaternion elements that
     * specify the orientation of the camera in the external frame.
     *
     * @param derivs
     *  Pointer to the start of the array of double values that on exit will contain the partial
     * derivatives of the distortion offset in (i,j) with respect to the extrinsic parameters. There are
     * eight elements, being the derivatives of the distortion offset in (i,j) with respect to the four
     * quaternion elements.
     * @param r_sez
     *  Position vector of the point in the SEZ frame.
     * @param q_sez_cam
     *  The unit quaternion that rotates vectors from the SEZ frame to the CAM frame.
     */
    void getForwardDistortionExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_sez, const Eigen::Quaterniond &q_sez_cam) const;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PinholeCamera);
        ar & BOOST_SERIALIZATION_NVP(d0);
        ar & BOOST_SERIALIZATION_NVP(d1);
        ar & BOOST_SERIALIZATION_NVP(d2);
        ar & BOOST_SERIALIZATION_NVP(d3);
        ar & BOOST_SERIALIZATION_NVP(d4);
        ar & BOOST_SERIALIZATION_NVP(d5);
        ar & BOOST_SERIALIZATION_NVP(d6);
        ar & BOOST_SERIALIZATION_NVP(e0);
        ar & BOOST_SERIALIZATION_NVP(e1);
        ar & BOOST_SERIALIZATION_NVP(e2);
        ar & BOOST_SERIALIZATION_NVP(e3);
        ar & BOOST_SERIALIZATION_NVP(e4);
        ar & BOOST_SERIALIZATION_NVP(e5);
        ar & BOOST_SERIALIZATION_NVP(e6);
        this->init();
    }

};

#endif // PINHOLECAMERAWITHRADIALDISTORTION_H
