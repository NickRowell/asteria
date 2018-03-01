#ifndef PINHOLECAMERAWITHRADIALANDTANGENTIALDISTORTION_H
#define PINHOLECAMERAWITHRADIALANDTANGENTIALDISTORTION_H

#include "optics/pinholecamera.h"

/**
 * @brief The PinholeCameraWithRadialAndTangentialDistortion class provides an implementation of the
 * CameraModelBase for modelling pinhole cameras with low order radial and tangential distortion.
 */
class PinholeCameraWithRadialAndTangentialDistortion : public PinholeCamera {

public:

    /**
     * @brief Default constructor for the PinholeCameraWithRadialAndTangentialDistortion.
     */
    PinholeCameraWithRadialAndTangentialDistortion();

    /**
     * @brief Main constructor for the PinholeCameraWithRadialAndTangentialDistortion.
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
    PinholeCameraWithRadialAndTangentialDistortion(const unsigned int &width, const unsigned int &height, const double &fi, const double &fj, const double &pi, const double &pj,
                                      const double &k2, const double &p1, const double &p2);

    ~PinholeCameraWithRadialAndTangentialDistortion();

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

    /**
     * @brief Enumerates the possible signs of the radial distortion.
     */
    enum RADIAL_DISTORTION {
        /** Points are distorted AWAY FROM the principal point */
        POSITIVE,
        /** Points are distorted TOWARDS the principal point */
        NEGATIVE,
        /** No distortion */
        ZERO
    };

    /**
     * @brief Sign of the radial distortion.
     */
    RADIAL_DISTORTION CURRENT_RADIAL_DISTORTION;

    PinholeCamera * convertToPinholeCamera() const;

    PinholeCameraWithRadialDistortion * convertToPinholeCameraWithRadialDistortion() const;

    PinholeCameraWithRadialAndTangentialDistortion * convertToPinholeCameraWithRadialAndTangentialDistortion() const;

    unsigned int getNumParameters() const;

    void getParameters(double *params) const;

    void getIntrinsicPartialDerivatives(double * derivs, const Eigen::Vector3d & r_cam) const;

    void getExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_sez, const Eigen::Quaterniond &q_sez_cam) const;

    void setParameters(const double *);

    Eigen::Vector3d deprojectPixel(const double & i, const double & j) const;

    bool projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const;

    std::string getModelName() const;

    /**
     * Computes the magnitude of the radial distortion at the given distance
     * from the principal point, i.e. returns C(R) where
     *
     * R' = C(R)*R
     *
     * Note that the value of R can be anywhere from zero (at the distortion
     * centre) to around 1000 (at the image edge), which means that the
     * powers of R used in the distortion factor polynomial can be huge. This
     * can cause problems with floating point errors, as the polynomial
     * coefficients are often very small numbers. Therefore we normalise the
     * radial distance and distortion coefficients using the camera focal length
     * in order to bring both their values closer to one, thus reducing
     * floating point errors.
     *
     * @param R
     *  Radial distance from the principal point of the pixel in the undistorted
     * image.
     * @return
     *  The forward radial distortion factor C(R)
     */
    double getForwardRadialDistortionFactor(const double &R) const;

    /**
     * This function computes the inverse radial distortion factor as a function
     * of radial distance, i.e. returns D(R') where
     *
     * R = D(R')*R'
     *
     * Note:
     * The camera model specifies the forward distortion (ideal -> distorted)
     * only; this is modelled as a polynomial in the radial distance
     * of the ideal point from the projection centre. Inverting this is very
     * difficult for anything other than very low order polynomials, basically
     * because the forward distortion model gives us the magnitude of the
     * distortion at the ideal (undistorted) point, but the inverse distortion
     * is applied to the distorted point at which the magnitude of the
     * distortion is not known.
     *
     * For forward radial distortion of the form (1 + k1*R^2), the analytic
     * inverse involves finding the roots of a cubic polynomial. For radial
     * distortion of the form (1 + k1*R^2 + k2*R^4), the roots of a quintic
     * polynomial are required which in general has no closed form solution.
     *
     * HOWEVER - it is possible to compute the inverse radial distortion in a
     * numerical manner with relative ease, including for high order distortion
     * models. This is the approach taken here. The iterative scheme for
     * estimating the inverse radial distortion works as follows:
     *
     * The (known) forward distortion model C allows us to obtain the distorted
     * radial distance R' in terms of the undistorted radial distance R like so:
     *
     * -> R' = C(R)*R
     *
     * The (unknown) inverse distortion model D maps the distorted radial
     * distance R' to the corresponding undistorted distance R like so:
     *
     * -> R = D(R')*R'
     *
     * We wish to estimate R at a specific value of R'. Notice that:
     *
     * -> R = R' / C(R)
     *
     * We therefore make an initial guess for R:
     *
     * R_0 = R' / C(R')
     *
     * ...then iteratively refine to converge on R using the following scheme:
     *
     * while(R_i not converged)
     * {
     *
     *     C(R_i) = 1 + k1 * R_i^2 + k2 * R_i^4 + ...
     *
     *     R_i+1 = 0.5 * (R_i + R'/C(R_i))
     *
     *     (compare to R_i to check for convergence)
     *
     * }
     *
     * On output, this gives us the undistorted radius R (as R_i) as a function
     * of the distorted radius R'. We obtain D(R') from R/R'.
     *
     * Note: various sources suggest using simply R_i+1 = R'/C(R_i) as the
     * update. However, I found that when the initial distortion C(R') is large,
     * the algorithm can take a long time to converge and also risks stalling on
     * a pair of R_i and R_i+1 such that:
     *
     *      R_i+1 = R_prime/C(R_i)        R_i = R_prime/C(R_i+1)
     *
     * In this case, the algorithm will simply jump back and forth between the
     * two values. I found empirically that taking the mean of R_i and R'/C(R_i)
     * avoided this and provided much faster convergence.
     *
     * @param R_prime
     *  Radial distance of the pixel from the principal point in the distorted
     * image, i.e. the detector.
     * @param tol
     *  Estimation is iterated until this tolerance is reached, i.e. undistorted
     * radial distance is accurate to within this number of pixels.
     * @return
     *  D(R_prime) if distortion factor is found within the set number of iterations,
     * -1 if not: this means the distortion is positive and too strong, i.e. the
     * distortion factor has become negative within the distorted image area.
     */
    double getBackwardRadialDistortionFactor(const double &R_prime, const double &tol) const;

    /**
     * @brief Computes the radially-distorted coordinates of a pixel from the undistorted coordinates.
     * Note that the distortion centre is located at the principal point.
     *
     * @param i
     *  i coordinate of undistorted pixel [pixels]
     * @param j
     *  j coordinate of undistorted pixel [pixels]
     * @param ip
     *  On exit, contains i coordinate of distorted pixel [pixels]
     * @param jp
     *  On exit, contains j coordinate of distorted pixel [pixels]
     */
    void getDistortedPixel(const double &i, const double &j, double &ip, double &jp) const;

    /**
     * @brief Computes the undistorted coordinates of a pixel from the radially-distorted coordinates.
     * Note that the distortion centre is located at the principal point.
     *
     * @param ip
     *  i coordinate of distorted pixel [pixels]
     * @param jp
     *  j coordinate of distorted pixel [pixels]
     * @param i
     *  On exit, contains i coordinate of undistorted pixel [pixels]
     * @param j
     *  On exit, contains j coordinate of undistorted pixel [pixels]
     */
    void getUndistortedPixel(const double &ip, const double &jp, double &i, double &j) const;

    void init();

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
