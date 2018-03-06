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
     * @param k2
     *  Second-order coefficient of the radial distortion polynomal [pixels\f$^{-2}\f$]
     * @param p1
     *  First coefficient of the tangential distortion model.
     * @param p2
     *  Second coefficient of the tangential distortion model.
     */
    PinholeCameraWithSipDistortion(const unsigned int &width, const unsigned int &height, const double &fi, const double &fj, const double &pi, const double &pj,
                                      const double &k2, const double &p1, const double &p2);

    ~PinholeCameraWithSipDistortion();

    /**
     * @brief Second-order coefficient of the forward radial distortion polynomial [pixels\f$^{-2}\f$].
     *
     * The forward radial distortion factor is computed according to:
     *
     * \f$C(R) = 1 + K2*R^2 + ... \f$
     *
     * where \f$R\f$ is measured from the distortion centre, which coincides with the principal point.
     * Note that the distortion factor is dimensionless. The distortion polynomial can be extended with
     * higher order terms with a bit of ajustment to the partial derivatives. It is unwise to add a
     * constant (K0) term because this is degenerate with the focal length. Higher order terms can be
     * difficult to constrain and lead to quite unphysical distortion functions. In general the function
     * should be monotonic, which is difficult (read: impossible) to apply rigorously in the fitting but
     * which seems to emerge naturally when fitting polynomials with few terms.
     */
    double K2;

    /**
     * @brief First coefficient of the tangential distortion model.
     */
    double p1;

    /**
     * @brief Second coefficient of the tangential distortion model.
     */
    double p2;

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
        ar & BOOST_SERIALIZATION_NVP(K2);
        ar & BOOST_SERIALIZATION_NVP(p1);
        ar & BOOST_SERIALIZATION_NVP(p2);
        this->init();
    }

};

#endif // PINHOLECAMERAWITHRADIALDISTORTION_H
