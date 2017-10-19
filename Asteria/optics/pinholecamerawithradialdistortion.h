#ifndef PINHOLECAMERAWITHRADIALDISTORTION_H
#define PINHOLECAMERAWITHRADIALDISTORTION_H

#include "optics/pinholecamera.h"

class PinholeCameraWithRadialDistortion : public PinholeCamera {

public:

    PinholeCameraWithRadialDistortion(const unsigned int &width, const unsigned int &height, const double &fi, const double &fj, const double &pi, const double &pj,
                                      const double &k0, const double &k1, const double &k2, const double &k3, const double &k4);
    ~PinholeCameraWithRadialDistortion();

    /**
     * Coefficients of FORWARD radial distortion polynomial (undistorted ->
     * distorted coordinates). These are the parameters that are estimated in
     * standard camera calibration reports.
     *
     * The forward radial distortion factor is computed according to:
     *
     * C(R) = 1 + K0 + K1*R + K2*R^2 + K3*R^3 + K4*R^4
     *
     * Note that the distortion factor is dimensionless, so the units on
     * the coefficients are as follows:
     *
     * K0 [-]
     * K1 [pixels^{-1}]
     * K2 [pixels^{-2}]
     * K3 [pixels^{-3}]
     * K4 [pixels^{-4}]
     */
    double K0, K1, K2, K3, K4;

    /**
     * @brief Enumerates the possible signs of the radial distortion.
     */
    enum RADIAL_DISTORTION {
        // Points are distorted AWAY FROM the principal point
        POSITIVE,
        // Points are distorted TOWARDS the principal point
        NEGATIVE,
        // No distortion
        ZERO
    };

    /**
     * @brief Sign of the radial distortion.
     */
    RADIAL_DISTORTION CURRENT_RADIAL_DISTORTION;

    double * getParameters(unsigned int &);

    void setParameters(double *);

    Eigen::Vector3d deprojectPixel(const double & i, const double & j) const;

    void projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const;

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
     *
     * @param R Radial distance from the principal point of the pixel in the
     *          undistorted image, i.e. the scene radiance.
     * @return
     */
    double getForwardRadialDistortion(const double &R) const;

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
     * @param R_prime   Radial distance of the pixel from the principal point
     *                  in the distorted image, i.e. the detector.
     * @param tol       Estimation is iterated until this tolerance is reached,
     *                  i.e. undistorted radial distance is accurate to within
     *                  this number of pixels.
     * @return          D(R_prime) if distortion factor is found within the set
     *                  number of iterations, -1 if not: this means the distortion
     *                  is positive and too strong, i.e. the distortion factor
     *                  has become negative within the distorted image area.
     */
    double getBackwardRadialDistortion(const double &R_prime, const double &tol) const;

    /**
     * @brief Computes the radially-distorted coordinates of a pixel from the undistorted coordinates.
     * Note that the distortion centre is located at the principal point.
     *
     * @param i  i coordinate of undistorted pixel [pixels]
     * @param j  j coordinate of undistorted pixel [pixels]
     * @param ip On exit, contains i coordinate of distorted pixel [pixels]
     * @param jp On exit, contains j coordinate of distorted pixel [pixels]
     */
    void getDistortedPixel(const double &i, const double &j, double &ip, double &jp) const;

    /**
     * @brief Computes the undistorted coordinates of a pixel from the radially-distorted coordinates.
     * Note that the distortion centre is located at the principal point.
     *
     * @param ip i coordinate of distorted pixel [pixels]
     * @param jp j coordinate of distorted pixel [pixels]
     * @param i  On exit, contains i coordinate of undistorted pixel [pixels]
     * @param j  On exit, contains j coordinate of undistorted pixel [pixels]
     */
    void getUndistortedPixel(const double &ip, const double &jp, double &i, double &j) const;

};

#endif // PINHOLECAMERAWITHRADIALDISTORTION_H
