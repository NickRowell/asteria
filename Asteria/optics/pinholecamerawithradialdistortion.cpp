#include "optics/pinholecamerawithradialdistortion.h"

BOOST_CLASS_EXPORT(PinholeCameraWithRadialDistortion)

PinholeCameraWithRadialDistortion::PinholeCameraWithRadialDistortion()  :
    PinholeCamera(), K0(0.0), K1(0.0), K2(0.0), K3(0.0), K4(0.0) {

}

PinholeCameraWithRadialDistortion::PinholeCameraWithRadialDistortion(const unsigned int &width, const unsigned int &height, const double &fi,
    const double &fj, const double &pi, const double &pj, const double &k0, const double &k1, const double &k2, const double &k3, const double &k4) :
    PinholeCamera(width, height, fi, fj, pi, pj), K0(k0), K1(k1), K2(k2), K3(k3), K4(k4) {
    init();
}

void PinholeCameraWithRadialDistortion::init() {

    // Call init() of superclass
    PinholeCamera::init();

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

    if((K0==0.0)&&(K1==0.0)&&(K2==0.0)&&(K3==0.0)&&(K4==0.0)) {
        CURRENT_RADIAL_DISTORTION = ZERO;
    }
    if((K0<=0.0)&&(K1<=0.0)&&(K2<=0.0)&&(K3<=0.0)&&(K4<=0.0)) {
        // (all negative or negative/zero mix)
        CURRENT_RADIAL_DISTORTION = NEGATIVE;
    }
    else if((K0>=0.0)&&(K1>=0.0)&&(K2>=0.0)&&(K3>=0.0)&&(K4>=0.0)) {
        // (all positive or positive/zero mix)
        CURRENT_RADIAL_DISTORTION = POSITIVE;
    }
    else {
        // Encountered failure condition
        fprintf(stderr, "Radial distortion parameters must be all the same sign or zero!\n");
        return;
    }

    // Get the maximum radial distance of any pixel from the projection
    // centre, in the detector image. This calculation is easy as we know
    // the detector size and the distortion centre (coincident with
    // projection centre).

    // Radial distance of extreme corners of detector image from distortion centre.
    float rp_tl = std::sqrt((pi)*(pi) + (pj)*(pj));                             // Top left
    float rp_tr = std::sqrt((width-pi)*(width-pi) + (pj)*(pj));                 // Top right
    float rp_bl = std::sqrt((pi)*(pi) + (height-pj)*(height-pj));               // Bottom left
    float rp_br = std::sqrt((width-pi)*(width-pi) + (height-pj)*(height-pj));   // Bottom right

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

        case ZERO: {
            break;
        }
        // This checks the validity of the (positive) radial distortion
        // coefficients, which amounts to ensuring that the backwards
        // radial distortion factor is positive over the full extent of
        // the distorted image.
        case POSITIVE: {
            // We already know what the maximum radial distance in the
            // distorted image is, so we only need to check that the
            // backwards radial distortion is computable at this point.

            // Tolerance on solution for backwards radial distortion. Can
            // be rough here as we're just testing the sign of the distortion
            // factor, and it's precise value is not important.
            float tolerance = 0.01;

            if(getBackwardRadialDistortionFactor(rp_max, tolerance) < 0) {
                // Encountered failure condition
                fprintf(stderr, "Positive radial distortion is too strong! Reduce coefficients.\n");
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
        case NEGATIVE: {
            // Loop over radial distance in undistorted image
            for(float r=0, rp, cr; ; r++) {
                // Find forwards distortion factor (u->d) at radial distance
                // r in undistorted image.
                cr = getForwardRadialDistortionFactor(r);

                // If distortion factor becomes negative or zero before loop
                // is exited, then distortion is too large.
                if(cr<=0.0) {
                    // Encountered failure condition
                    fprintf(stderr, "Negative radial distortion is too strong! Reduce coefficients.\n");
                    return;
                }

                // Calculate corresponding distorted radial distance
                rp = cr*r;

                // If this is larger than the largest radial distance observed in
                // the detector image, then we have verified that distortion is
                // within allowed range.
                if(rp > rp_max) {
                    r_max = r;
                    break;
                }
            }

            break;
        }
    }
}

PinholeCameraWithRadialDistortion::~PinholeCameraWithRadialDistortion() {

}

unsigned int PinholeCameraWithRadialDistortion::getNumParameters() const {
    return 9;
}

void PinholeCameraWithRadialDistortion::getParameters(double * params) const {
    params[0] = fi;
    params[1] = fj;
    params[2] = pi;
    params[3] = pj;
    params[4] = K0;
    params[5] = K1;
    params[6] = K2;
    params[7] = K3;
    params[8] = K4;
}

void PinholeCameraWithRadialDistortion::getIntrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_cam) const {

    double x_cam = r_cam[0];
    double y_cam = r_cam[1];
    double z_cam = r_cam[2];

    // Get the ideal (undistorted) image coordinates
    double i, j;
    PinholeCamera::projectVector(r_cam, i, j);

    // Distance of the undistorted point from the distortion (projection) centre;
    // determines the magnitude of the radial distortion
    double R = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));

    // NOTE that dR/dpi = 0.0 and dR/dpj = 0.0 due to dependence of i/j on pi/pj which cancel out
    double dR_dfi = fi * (x_cam/z_cam) * (x_cam/z_cam) / R;
    double dR_dfj = fj * (y_cam/z_cam) * (y_cam/z_cam) / R;

    // Magnitude of the radial distortion
    double CR = getForwardRadialDistortionFactor(R);

    // Partial derivatives of radial distortion factor
    // CR = 1 + K0 + K1*R + K2*R^2 + K3*R^3 + K4*R^4
    // Derivatives wrt fi,fj
    double dcr_dfi = K1*dR_dfi + 2*K2*R*dR_dfi* + 3*K3*R*R*dR_dfi + 4*K4*R*R*R*dR_dfi;
    double dcr_dfj = K1*dR_dfj + 2*K2*R*dR_dfj* + 3*K3*R*R*dR_dfj + 4*K4*R*R*R*dR_dfj;
    // Derivatives wrt pi,pj are zero due to dR/dpi and dR/dpj = 0.0
    // Derivatives wrt K0->K4
    double dcr_dK0 = 1.0;
    double dcr_dK1 = R;
    double dcr_dK2 = R*R;
    double dcr_dK3 = R*R*R;
    double dcr_dK4 = R*R*R*R;

    // i = fi * (x_cam/z_cam) + pi
    // i' = (i - pi) * C(R) + pi = fi * (x_cam/z_cam) * C(R) + pi
    // j = fj * (y_cam/z_cam) + pj
    // j' = (j - pj) * C(R) + pj = fj * (y_cam/z_cam) * C(R) + pj
    // with R = sqrt((i-pi)^2 + (j-pj)^2)

    // di'/dfi = [x_cam / z_cam] * C(R) + fi * [x_cam / z_cam] * dCR/dfi
    derivs[0] = (x_cam/z_cam) * CR + fi * (x_cam/z_cam) * dcr_dfi;
    // dj'/dfi = fj * [y_cam / z_cam] * dCR/dfi
    derivs[1] = fj * (y_cam/z_cam) * dcr_dfi;

    // di'/dfj = fi * [x_cam / z_cam] * dCR/dfj
    derivs[2] = fi * (x_cam/z_cam) * dcr_dfj;
    // dj'/dfj = [y_cam / z_cam] * C(R) + fj * [y_cam / z_cam] * dCR/dfj
    derivs[3] = (y_cam/z_cam) * CR + fj * (y_cam/z_cam) * dcr_dfj;

    // di'/dpi = 1.0 (note that dC(R)/dpi = 0.0)
    derivs[4] = 1.0;
    // dj'/dpi = 0.0 (note that dC(R)/dpi = 0.0)
    derivs[5] = 0.0;

    // di'/dpj = 0.0 (note that dC(R)/dpj = 0.0)
    derivs[6] = 0.0;
    // dj'/dpj = 1.0 (note that dC(R)/dpj = 0.0)
    derivs[7] = 1.0;

    // di'/dK0 = [x_cam / z_cam] * fi * dC(R)/dK0
    derivs[8] = (x_cam/z_cam) * fi * dcr_dK0;
    // dj'/dK0 = [y_cam / z_cam] * fj * dC(R)/dK0
    derivs[9] = (y_cam/z_cam) * fj * dcr_dK0;

    // di'/dK1 = [x_cam / z_cam] * fi * dC(R)/dK1
    derivs[10] = (x_cam/z_cam) * fi * dcr_dK1;
    // dj'/dK1 = [y_cam / z_cam] * fj * dC(R)/dK1
    derivs[11] = (y_cam/z_cam) * fj * dcr_dK1;

    // di'/dK2 = [x_cam / z_cam] * fi * dC(R)/dK2
    derivs[12] = (x_cam/z_cam) * fi * dcr_dK2;
    // dj'/dK2 = [y_cam / z_cam] * fj * dC(R)/dK2
    derivs[13] = (y_cam/z_cam) * fj * dcr_dK2;

    // di'/dK3 = [x_cam / z_cam] * fi * dC(R)/dK3
    derivs[14] = (x_cam/z_cam) * fi * dcr_dK3;
    // dj'/dK3 = [y_cam / z_cam] * fj * dC(R)/dK3
    derivs[15] = (y_cam/z_cam) * fj * dcr_dK3;

    // di'/dK4 = [x_cam / z_cam] * fi * dC(R)/dK4
    derivs[16] = (x_cam/z_cam) * fi * dcr_dK4;
    // dj'/dK4 = [y_cam / z_cam] * fj * dC(R)/dK4
    derivs[17] = (y_cam/z_cam) * fj * dcr_dK4;
}

void PinholeCameraWithRadialDistortion::getExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d &r_sez, const Eigen::Matrix3d &r_sez_cam) const {

    // The derivatives are the same as those for the pinhole camera, but multiplied by the forward radial distortion factor
    PinholeCamera::getExtrinsicPartialDerivatives(derivs, r_sez, r_sez_cam);

    // Now compute the forward radial distortion factor...

    // Get the position vector in the camera frame
    Eigen::Vector3d r_cam = r_sez_cam * r_sez;

    // Get the ideal (undistorted) image coordinates
    double i, j;
    PinholeCamera::projectVector(r_cam, i, j);

    // Distance of the undistorted point from the distortion (projection) centre;
    // determines the magnitude of the radial distortion
    double R = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));

    // Magnitude of the radial distortion
    double CR = getForwardRadialDistortionFactor(R);

    // Scale the partial derivatives accordingly...
    derivs[0] *= CR;
    derivs[1] *= CR;
    derivs[2] *= CR;
    derivs[3] *= CR;
    derivs[4] *= CR;
    derivs[5] *= CR;
    derivs[6] *= CR;
    derivs[7] *= CR;
}

void PinholeCameraWithRadialDistortion::setParameters(const double *params) {
    fi = params[0];
    fj = params[1];
    pi = params[2];
    pj = params[3];
    K0 = params[4];
    K1 = params[5];
    K2 = params[6];
    K3 = params[7];
    K4 = params[8];
}

Eigen::Vector3d PinholeCameraWithRadialDistortion::deprojectPixel(const double & i, const double & j) const {

    // Remove the radial distortion to get the undistorted pixel coordinates
    double i_ideal, j_ideal;
    getUndistortedPixel(i, j, i_ideal, j_ideal);

    // Use function in superclass to deproject undistorted pixel coordinates
    return PinholeCamera::deprojectPixel(i_ideal, j_ideal);
}

void PinholeCameraWithRadialDistortion::projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const {

    // Use function in superclass to project vector to undistorted pixel coordinates
    double i_ideal, j_ideal;
    PinholeCamera::projectVector(r_cam, i_ideal, j_ideal);

    // Apply radial distortion
    getDistortedPixel(i_ideal, j_ideal, i, j);
}

std::string PinholeCameraWithRadialDistortion::getModelName() const {
    return "PinholeCameraWithRadialDistortion";
}

double PinholeCameraWithRadialDistortion::getForwardRadialDistortionFactor(const double &R) const {

    // Use mean focal length to normalise quantities and keep numbers low
    double f = (fi + fj)/2.0;

    // Normalise radial distance and polynomial coefficients
    double Rn = R/f;

    double K0n = K0;
    double K1n = K1*f;
    double K2n = K2*f*f;
    double K3n = K3*f*f*f;
    double K4n = K4*f*f*f*f;

    // Distortion factor: 1 + K0 + K1*R + K2*R^2 + K3*R^3 + K4*R^4
    // Probably more efficient this way, as it avoids computing large
    // powers directly:
    double CR = 1 + K0n + Rn*(K1n + Rn*(K2n + Rn*(K3n + Rn*K4n)));

    // Check on CR.
    // For negative distortion coefficients, CR is less than one. If the
    // distortion is too large, CR can drop below zero which causes points
    // to be reflected to negative radial distances. This is unphysical:
    // the radial distortion components need to be specified by the user so
    // that this does not happen.
    if(CR<=0) {
        fprintf(stderr, "Forward radial distortion factor negative!\n");
    }

    return CR;
}

double PinholeCameraWithRadialDistortion::getBackwardRadialDistortionFactor(const double &R_prime, const double &tol) const {

    // Check for no distortion
    if(R_prime==0.0) {
        return 1.0;
    }
    // Initial guess R_0
    double R_i = R_prime;
    // Intermediate variables
    double R_ip1, CR_i;
    // Iterations limit
    unsigned int MAX_ITERATIONS=1000;

    // Loop until converged
    while(MAX_ITERATIONS-- > 0) {

        // Computes 1+K0+K1R+...
        CR_i = getForwardRadialDistortionFactor(R_i);
        // Update step
        R_ip1 = 0.5*(R_i + R_prime/CR_i);


        if(fabs(R_ip1 - R_i) < tol) {
            // Converged
            return R_ip1/R_prime;
        }
        else {
            // Not converged
            R_i = R_ip1;
        }
    }

    // If we have not converged to a solution within the allocated
    // number of iterations, then this indicates that the backwards
    // radial distortion factor is negative at this value of R_prime.
    // This occurs for positive radial distortion, when the coefficients
    // are too large. We return a negative value to indicate that
    // this is the case.
    return -1.0;
}


void PinholeCameraWithRadialDistortion::getDistortedPixel(const double &i, const double &j, double &ip, double &jp) const {

    double R = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));

    // Computes 1 + K0 + K1*R + K2*R^2 + K3*R^3 + K4*R^4
    double CR = getForwardRadialDistortionFactor(R);

    ip = (i - pi)*CR + pi;
    jp = (j - pj)*CR + pj;
}

void PinholeCameraWithRadialDistortion::getUndistortedPixel(const double &ip, const double &jp, double &i, double &j) const {
    double rp = std::sqrt((ip-pi)*(ip-pi) + (jp-pj)*(jp-pj));

    double Drp = getBackwardRadialDistortionFactor(rp, 0.01);

    i = (ip - pi)*Drp + pi;
    j = (jp - pj)*Drp + pj;
}
