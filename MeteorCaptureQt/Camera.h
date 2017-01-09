/*
 * Camera.h
 *
 * Camera class will store all the calibration fields of the camera and provide functions
 * to project/deproject vectors/points etc.
 *
 *
 *  Created on: 16 Dec 2016
 *      Author: nrowell
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <cassert>

using namespace std;

template < typename T >
class CameraModel {

public:

    /**
     * DERIVED PARAMETER.
     * This flag indicates whether we have been able to use the user-specified
     * parameters to create a valid CameraModel object. There are various
     * pitfalls, such as specifying invalid radial distortion coefficients, that
     * may prevent the construction of a valid camera model.
     * We initialise the value to false, and exit the CameraModel constructor
     * early if a failure condition is found. Otherwise, we set this flag to
     * true at the end of the constructor.
     */
    bool valid;

    /**
     * Camera projection matrix K. This must have the form:
     *
     *      fi   0   pi
     * K =   0  fj   pj
     *       0   0    1
     *
     * where fi, fj are the focal lengths and (pi,pj) is the coordinate of
     * the principal point. Note that a non-zero skew parameter (element 01)
     * is not supported.
     */
    Mat3x3<T> K;

    /**
     * Width and height of the detector [pixels].
     */
    unsigned int width;
    unsigned int height;

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
    T K0, K1, K2, K3, K4;

    /**
     * DERIVED PARAMETER.
     * Inverse of camera matrix.
     */
    Mat3x3<T>  invK;

    /**
     * DERIVED PARAMETER.
     * Sense of the radial distortion.
     */
    enum RADIAL_DISTORTION{ POSITIVE, NEGATIVE, ZERO};
    RADIAL_DISTORTION CURRENT_RADIAL_DISTORTION;

    /**
     * USER PARAMETER.
     * Exposure time [seconds].
     */
    float exposure_time;

    bool exposureTimeIsConfigurable;
    float minExp, maxExp;



    /**
     * Constructor for user-defined camera model.
     * @param pK
     * @param pwidth
     * @param pheight
     */
    CameraModel(const Mat3x3<T> &pK,
                const unsigned int & pwidth,
                const unsigned int & pheight,
                T pK0, T pK1, T pK2, T pK3, T pK4,
                const float & pexposure_time) :
                K(pK), width(pwidth), height(pheight),
                K0(pK0), K1(pK1), K2(pK2), K3(pK3), K4(pK4),
                exposure_time(pexposure_time)
    {

        // Set flag to false initially. If we make it to the end of the
        // constructor without encountering any failure conditions, then the
        // flag is reset to true.
        valid = false;

        // Get some useful values from pinhole camera matrix
        float fi   = K.get(0,0);  // Focal lengths
        float fj   = K.get(1,1);
        float pi   = K.get(0,2);  // Principal point/projection centre coordinates
        float pj   = K.get(1,2);

        // Principal point must lie in image boundary. Otherwise calculation of
        // distortion factor range and undistorted image size will fail. This
        // constraint could be relaxed by improving the calculation of the
        // undistorted image to consider the distortion along all image boundaries.
        // This may be necessary for tiled rendering, or to support CCD arrays.
        if(pi < 0 || pi > width || pj < 0 || pj > height)
        {
            // Encountered failure condition
            std::cerr << "Principal point must lie inside image boundary!\n";
            return;
        }

        // Get inverse matrix for doing back-projections. This calculation
        // exploits the known structure of the camera matrix K, assuming
        // zero skew parameter:
        invK.init(1/fi, 0, -pi/fi, 0, 1/fj, -pj/fj, 0, 0, 1);

        // RADIAL DISTORTION

        // Check that all distortion parameters have the same sign. This is
        // necessary to ensure that radial distortion is a monotonic function
        // over all values of R (i.e. positive - we ignore negative values of R
        // which are unphysical. The radial distortion function only needs to be
        // monotonic for positive values of R).
        // This condition is perhaps too strict - for example, a mix of positive
        // and negative parameters can still provide a monotonic distortion
        // function within the image area. However automatically checking for
        // this is difficult, so we just insist that all coefficients have the
        // same sign, which guarantees a monotonic function (for positive R).

        if((K0==0.0)&&(K1==0.0)&&(K2==0.0)&&(K3==0.0)&&(K4==0.0))
        {
            CURRENT_RADIAL_DISTORTION = ZERO;
        }
        if((K0<=0.0)&&(K1<=0.0)&&(K2<=0.0)&&(K3<=0.0)&&(K4<=0.0))
        {
            // (all negative or negative/zero mix)
            CURRENT_RADIAL_DISTORTION = NEGATIVE;
        }
        else if((K0>=0.0)&&(K1>=0.0)&&(K2>=0.0)&&(K3>=0.0)&&(K4>=0.0))
        {
            // (all positive or positive/zero mix)
            CURRENT_RADIAL_DISTORTION = POSITIVE;
        }
        else
        {
            // Encountered failure condition
            std::cerr << "Radial distortion parameters must be all the same "
                      << "sign or zero!\n";
            return;
        }

        // Get the maximum radial distance of any pixel from the projection
        // centre, in the detector image. This calculation is easy as we know
        // the detector size and the distortion centre (coincident with
        // projection centre).

        // Radial distance of extreme corners of detector image from distortion centre.
        float rp_tl = sqrt((pi)*(pi) + (pj)*(pj));                             // Top left
        float rp_tr = sqrt((width-pi)*(width-pi) + (pj)*(pj));                 // Top right
        float rp_bl = sqrt((pi)*(pi) + (height-pj)*(height-pj));               // Bottom left
        float rp_br = sqrt((width-pi)*(width-pi) + (height-pj)*(height-pj));   // Bottom right

        // Maximum radial distance of any detector pixel from distortion centre.
        float rp_max = std::max(std::max(rp_tl,rp_bl) , std::max(rp_tr,rp_br));

        // Add a small fudge factor so that we calculate the lookup tables slightly
        // beyond the required range. This is because we have to un-distort the
        // extreme corners of the pixel array when computing the solid angles of
        // the corner pixels, and floating point errors can result in overshooting
        // the end of the distortion factor lookup table in the detector fragment
        // shader.
        rp_max += 0.1;

        // This section checks that the radial distortion coefficients are
        // valid, and determines the range of radial distance in the
        // undistorted image. Note that:
        // 1) we know the range in radial distance in the distorted image,
        //    because this is just the maximum distance of any point in the
        //    detector pixel array from the distortion centre
        // 2) we find the range in radial distance in the undistorted image
        //    by measuring the corresponding backwards radial distortion factor
        // 3) In the case of negative distortion, we must check that the forwards
        //    distortion factor remains positive at the extremes of the undistorted
        //    image.
        // 4) In the case of positive distortion, we must check that the backwards
        //    distortion factor remains positive at the extremes of the distorted
        //    image.
        //

        // Maximum radial distance of any undistorted pixel from distortion
        // centre. For positive distortion, we leave this equal to the maximum
        // distance in the detector image. Note that we could shave some borders off
        // but don't bother with this. The calculation in this case is trickier
        // because the border size is determined by the displacement of points along the image
        // edge that are distorted the least, so we need to consider the full
        // image boundary rather than just the corners (the displacement of which
        // determine the borders in the case of negative radial distortion).
        float r_max = rp_max;

        switch(CURRENT_RADIAL_DISTORTION)
        {

            case ZERO: {break;}
            // This checks the validity of the (positive) radial distortion
            // coefficients, which amounts to ensuring that the backwards
            // radial distortion factor is positive over the full extent of
            // the distorted image.
            case POSITIVE:
            {
                // We already know what the maximum radial distance in the
                // distorted image is, so we only need to check that the
                // backwards radial distortion is computable at this point.

                // Tolerance on solution for backwards radial distortion. Can
                // be rough here as we're just testing the sign of the distortion
                // factor, and it's precise value is not important.
                float tolerance = 0.01;

                if(getBackwardRadialDistortion(rp_max, tolerance) < 0)
                {
                    // Encountered failure condition
                    std::cerr << "Positive radial distortion is too strong!"
                              << " Reduce coefficients.\n";
                    return;
                }
                break;
            }
            // This checks the validity of the (negative) radial distortion
            // coefficients, and calculates the limits on the radial distance
            // in the distorted and undistorted images. These are used to
            // configure the scene radiance image camera.
            // The test here is that the forwards radial distortion factor
            // is positive over the full range of the undistorted image.
            case NEGATIVE:
            {
                // Loop over radial distance in undistorted image
                for(float r=0, rp, cr; ; r++)
                {
                    // Find forwards distortion factor (u->d) at radial distance
                    // r in undistorted image.
                    cr = getForwardRadialDistortion(r);

                    // If distortion factor becomes negative or zero before loop
                    // is exited, then distortion is too large.
                    if(cr<=0.0)
                    {
                        // Encountered failure condition
                        std::cerr << "Negative radial distortion is too strong!"
                                  << " Reduce coefficients.\n";
                        return;
                    }

                    // Calculate corresponding distorted radial distance
                    rp = cr*r;

                    // If this is larger than the largest radial distance observed in
                    // the detector image, then we have verified that distortion is
                    // within allowed range.
                    if(rp > rp_max)
                    {
                        r_max = r;
                        break;
                    }
                }

                break;
            }
        }

        // Knowing the limits on radial distance in both images, we can now
        // calculate 1D lookup tables for the forwards and backwards distortion
        // factors.
//        computeRadialDistortionLookupTables(rp_max, r_max);

        // All CameraModel parameters set correctly
        valid = true;

    };

    ~CameraModel() {}

    /**
     * Get the unit vector towards a given detector pixel
     * coordinate, correcting for any radial distortion.
     * \param ip   i coordinate of pixel in distorted image.
     * \param jp   j coordinate of pixel in distorted image.
     * \return     Camera frame unit vector towards the pixel.
     */
    Vec3<T> deprojectPixel(const T & ip, const T & jp) const
    {
        // Principal point/projection centre coordinates
        float i0   = K.get(0,2);
        float j0   = K.get(1,2);

        // Coordinates corrected for radial distortion
        float i, j;

        getUndistortedPixel(ip, jp, i0, j0, i, j);

        // Vector of length of 1 along optical axis (Z)
        Vec3<T> unitZ = invK*(Vec3<T>(i,j,1));

        // Unit vector
        unitZ = unitZ.normalise();

        return unitZ;
     }

    /**
     * Project the given camera frame position vector into the image plane,
     * applying forwards radial distortion.
     * \param X_vec     Camera frame position vector.
     * \param ip        On exit, contains the i image coordinates.
     * \param jp        On exit, contains the j image coordinates.
     * \return
     */
    void projectVector(const Vec3<T> & X_cam, T & ip, T & jp) const
    {
        // Homogenous vector in image plane
        Vec3<T> X_im = K*X_cam;

        // Ideal (undistorted) coordinates of point
        float i = X_im.x/X_im.z;
        float j = X_im.y/X_im.z;

        // Principal point/projection centre coordinates
        float i0   = K.get(0,2);
        float j0   = K.get(1,2);

        // Apply radial distortion
        getDistortedPixel(i, j, i0, j0, ip, jp);
    };

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
    float getForwardRadialDistortion(const float &R) const
    {
        // Use mean focal length to normalise quantities and
        // keep numbers low
        float f = (K.get(0,0) + K.get(1,1))/2.0;

        // Normalise radial distance and polynomial coefficients
        float Rn = R/f;

        float K0n = K0;
        float K1n = K1*f;
        float K2n = K2*f*f;
        float K3n = K3*f*f*f;
        float K4n = K4*f*f*f*f;

        // Distortion factor: 1 + K0 + K1*R + K2*R^2 + K3*R^3 + K4*R^4
        // Probably more efficient this way, as it avoids computing large
        // powers directly:
        float CR = (1 + K0n + Rn*(K1n + Rn*(K2n + Rn*(K3n + Rn*K4n))));

        // Check on CR.
        // For negative distortion coefficients, CR is less than one. If the
        // distortion is too large, CR can drop below zero which causes points
        // to be reflected to negative radial distances. This is unphysical:
        // the radial distortion components need to be specified by the user so
        // that this does not happen.
        if(CR<=0)
        {
            std::cerr << "Forward radial distortion factor negative!\n";
        }

        return CR;
    }

    /**
     * Computes the distorted coordinates of a pixel based on the ideal
     * (undistorted) coordinates, principal point coordinates, and forwards
     * radial distortion model.
     * @param i  i coordinate of undistorted pixel
     * @param j  j coordinate of undistorted pixel
     * @param i0 i coordinate of distortion centre
     * @param j0 j coordinate of distortion centre
     * @param ip On exit, contains i coordinate of distorted pixel
     * @param jp On exit, contains j coordinate of distorted pixel
     */
    void getDistortedPixel(const float &i, const float &j,
                           const float &i0, const float &j0,
                           float &ip, float &jp) const
    {
        float R = sqrt((i-i0)*(i-i0) + (j-j0)*(j-j0));

        // Computes 1 + K0 + K1*R + K2*R^2 + K3*R^3 + K4*R^4
        float CR = getForwardRadialDistortion(R);

        ip = (i - i0)*CR + i0;
        jp = (j - j0)*CR + j0;
    }

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
     * \param R_prime   Radial distance of the pixel from the principal point
     *                  in the distorted image, i.e. the detector.
     * \param tol       Estimation is iterated until this tolerance is reached,
     *                  i.e. undistorted radial distance is accurate to within
     *                  this number of pixels.
     * \return          D(R_prime) if distortion factor is found within the set
     *                  number of iterations, -1 if not: this means the distortion
     *                  is positive and too strong, i.e. the distortion factor
     *                  has become negative within the distorted image area.
     */
    float getBackwardRadialDistortion(const float &R_prime, const float &tol)
    {

        if(R_prime==0.0) return 1.0;                 // Check for no distortion
        float R_i = R_prime;                         // Initial guess R_0
        float R_ip1, CR_i;                           // Intermediate variables

        unsigned int MAX_ITERATIONS=1000;            // Iterations limit

        while(MAX_ITERATIONS-- > 0)                  // Loop until converged
        {
            CR_i = getForwardRadialDistortion(R_i);  // Computes 1+K0+K1R+...
            R_ip1 = 0.5*(R_i + R_prime/CR_i);        // Update step

            if(fabs(R_ip1 - R_i) < tol)
                return R_ip1/R_prime;                // Converged
            else
                R_i = R_ip1;                         // Not converged
        }

        // If we have not converged to a solution within the allocated
        // number of iterations, then this indicates that the backwards
        // radial distortion factor is negative at this value of R_prime.
        // This occurs for positive radial distortion, when the coefficients
        // are too large. We return a negative value to indicate that
        // this is the case.
        return -1.0;
    }

    /**
     * @param ip i coordinate of distorted pixel
     * @param jp j coordinate of distorted pixel
     * @param i0 i coordinate of distortion centre
     * @param j0 j coordinate of distortion centre
     * @param i  On exit, contains i coordinate of undistorted pixel
     * @param j  On exit, contains j coordinate of undistorted pixel
     */
    void getUndistortedPixel(const float &ip, const float &jp,
                             const float &i0, const float &j0,
                             float &i, float &j)
    {
        float rp = sqrt((ip-i0)*(ip-i0) + (jp-j0)*(jp-j0));

        float Drp = getBackwardRadialDistortion(rp, 0.01);

        i = (ip - i0)*Drp + i0;
        j = (jp - j0)*Drp + j0;
    }

    friend std::ostream& operator<<(std::ostream& os, const CameraModel& cam)
    {
        os << "\nDetector geometry:\n"
           << "\nPinhole camera matrix:\n" << cam.K
           << "\nInverse pinhole camera matrix:\n" << cam.invK
           << "\nImage dimensions: " << cam.width << " " << cam.height
           << "\nRadial distortion coefficients:"
           << "\nK0 = " << cam.K0
           << "\nK1 = " << cam.K1
           << "\nK2 = " << cam.K2
           << "\nK3 = " << cam.K3
           << "\nK4 = " << cam.K4
           << "\nExposure time [s] = " << cam.exposure_time
           << "\n\n";

        // Return a reference to the original ostream so that we can chain
        // stream insertion operations.
        return os;
    }
};

#endif /* CAMERA_H_ */
