#include "optics/pinholecamerawithradialdistortion.h"
#include "util/coordinateutil.h"

BOOST_CLASS_EXPORT(PinholeCameraWithRadialDistortion)

PinholeCameraWithRadialDistortion::PinholeCameraWithRadialDistortion()  :
    PinholeCamera(), K2(0.0) {

}

PinholeCameraWithRadialDistortion::PinholeCameraWithRadialDistortion(const unsigned int &width, const unsigned int &height, const double &fi,
    const double &fj, const double &pi, const double &pj, const double &k2) :
    PinholeCamera(width, height, fi, fj, pi, pj), K2(k2) {
    init();
}

PinholeCameraWithRadialDistortion::~PinholeCameraWithRadialDistortion() {

}

PinholeCamera * PinholeCameraWithRadialDistortion::convertToPinholeCamera() const {

    fprintf(stderr, "Converting a PinholeCameraWithRadialDistortion to a PinholeCamera\n");

    // Discard the distortion coefficients
    PinholeCamera * cam = new PinholeCamera(this->width, this->height, this->fi, this->fj, this->pi, this->pj);

    return cam;
}

PinholeCameraWithRadialDistortion * PinholeCameraWithRadialDistortion::convertToPinholeCameraWithRadialDistortion() const {

    fprintf(stderr, "Converting a PinholeCameraWithRadialDistortion to a PinholeCameraWithRadialDistortion\n");

    PinholeCameraWithRadialDistortion * cam = new PinholeCameraWithRadialDistortion(
                this->width, this->height, this->fi, this->fj, this->pi, this->pj, this->K2);

    return cam;
}

void PinholeCameraWithRadialDistortion::init() {

    // Call init() of superclass
    PinholeCamera::init();

    // RADIAL DISTORTION
    if((K2<0.0)) {
        CURRENT_RADIAL_DISTORTION = NEGATIVE;
    }
    else if((K2>0.0)) {
        CURRENT_RADIAL_DISTORTION = POSITIVE;
    }
    else {
        CURRENT_RADIAL_DISTORTION = ZERO;
    }

    // Compute the threshold on (undistorted) radial distance for a point to be visible

    // Radial distance of extreme corners of (undistorted) image from distortion centre.
    double rp_tl = std::sqrt((pi)*(pi) + (pj)*(pj));                             // Top left
    double rp_tr = std::sqrt((width-pi)*(width-pi) + (pj)*(pj));                 // Top right
    double rp_bl = std::sqrt((pi)*(pi) + (height-pj)*(height-pj));               // Bottom left
    double rp_br = std::sqrt((width-pi)*(width-pi) + (height-pj)*(height-pj));   // Bottom right

    // Maximum (undistorted) radial distance of any pixel from distortion centre.
    double rp_max = std::max(std::max(rp_tl,rp_bl) , std::max(rp_tr,rp_br));

    // Check the validity of the radial distortion coefficients, now that we know where
    // the largest distortion will occur. Note that:
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
    switch(CURRENT_RADIAL_DISTORTION) {

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
            if(getBackwardRadialDistortionFactor(rp_max, 0.01) < 0) {
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
                    break;
                }
            }

            break;
        }
    }

    // Compute r_max
    switch(CURRENT_RADIAL_DISTORTION) {

        case ZERO: {
            // No radial distortion, so distorted and undistorted points are the same.
            r_max = rp_max;
            break;
        }
        case POSITIVE: {
            // Positive radial distortion moves points away from the distortion centre, so
            // points slightly inside the FOV will be moved outside of it. That means that in
            // practise the maximum (undistorted) radial distance is slightly smaller than rp_max,
            // but we neglect that as only a rough check is required. This means that points
            // slightly outside the (distorted) FOV will pass the initial visibility check but
            // will fail the second check of the image coordinates.
            r_max = rp_max;
            break;
        }
        case NEGATIVE: {
            // Negative radial distortion moves points towards the distortion centre, so points
            // slightly outside of the FOV are moved inside it. This means that the maximum (undistorted)
            // radial distance is slightly larger than rp_max, and we need to adjust accordingly
            // otherwise we'll incorrectly flag points at the images edges/corners as not visible.
            r_max = rp_max * getBackwardRadialDistortionFactor(rp_max, 0.01);
        }
    }

}

unsigned int PinholeCameraWithRadialDistortion::getNumParameters() const {
    return 5;
}

void PinholeCameraWithRadialDistortion::getParameters(double * params) const {
    params[0] = fi;
    params[1] = fj;
    params[2] = pi;
    params[3] = pj;
    params[4] = K2;
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
//    double R = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));
    double R = std::sqrt((x_cam/z_cam)*(x_cam/z_cam) + (y_cam/z_cam)*(y_cam/z_cam));

    // NOTE that dR/dpi = 0.0 and dR/dpj = 0.0 due to dependence of i/j on pi/pj which cancel out
//    double dR_dfi = fi * (x_cam/z_cam) * (x_cam/z_cam) / R;
//    double dR_dfj = fj * (y_cam/z_cam) * (y_cam/z_cam) / R;

    // Magnitude of the radial distortion
    double CR = getForwardRadialDistortionFactor(R);

    // Partial derivatives of radial distortion factor
    // CR = 1 + K1*R + K2*R^2 + K3*R^3 + K4*R^4
    // Derivatives wrt fi,fj
//    double dcr_dfi = K1*dR_dfi + 2*K2*R*dR_dfi* + 3*K3*R*R*dR_dfi + 4*K4*R*R*R*dR_dfi;
//    double dcr_dfj = K1*dR_dfj + 2*K2*R*dR_dfj* + 3*K3*R*R*dR_dfj + 4*K4*R*R*R*dR_dfj;
    // Derivatives wrt pi,pj are zero due to dR/dpi and dR/dpj = 0.0
    // Derivatives wrt K1->K4
//    double dcr_dK1 = R;
    double dcr_dK2 = R*R;
//    double dcr_dK3 = R*R*R;
//    double dcr_dK4 = R*R*R*R;

    // i = fi * (x_cam/z_cam) + pi
    // i' = (i - pi) * C(R) + pi = fi * (x_cam/z_cam) * C(R) + pi
    // j = fj * (y_cam/z_cam) + pj
    // j' = (j - pj) * C(R) + pj = fj * (y_cam/z_cam) * C(R) + pj
    // with R = sqrt((i-pi)^2 + (j-pj)^2)

    // di'/dfi = [x_cam / z_cam] * C(R) + fi * [x_cam / z_cam] * dCR/dfi
//    derivs[0] = (x_cam/z_cam) * CR + fi * (x_cam/z_cam) * dcr_dfi;
    derivs[0] = (x_cam/z_cam) * CR;

    // dj'/dfi = fj * [y_cam / z_cam] * dCR/dfi
//    derivs[1] = fj * (y_cam/z_cam) * dcr_dfi;
    derivs[1] = 0.0;

    // di'/dfj = fi * [x_cam / z_cam] * dCR/dfj
//    derivs[2] = fi * (x_cam/z_cam) * dcr_dfj;
    derivs[2] = 0.0;

    // dj'/dfj = [y_cam / z_cam] * C(R) + fj * [y_cam / z_cam] * dCR/dfj
//    derivs[3] = (y_cam/z_cam) * CR + fj * (y_cam/z_cam) * dcr_dfj;
    derivs[3] = (y_cam/z_cam) * CR;

    // di'/dpi = 1.0 (note that dC(R)/dpi = 0.0)
    derivs[4] = 1.0;
    // dj'/dpi = 0.0 (note that dC(R)/dpi = 0.0)
    derivs[5] = 0.0;

    // di'/dpj = 0.0 (note that dC(R)/dpj = 0.0)
    derivs[6] = 0.0;
    // dj'/dpj = 1.0 (note that dC(R)/dpj = 0.0)
    derivs[7] = 1.0;

    // di'/dK1 = [x_cam / z_cam] * fi * dC(R)/dK1
//    derivs[8] = (x_cam/z_cam) * fi * dcr_dK1;
    // dj'/dK1 = [y_cam / z_cam] * fj * dC(R)/dK1
//    derivs[9] = (y_cam/z_cam) * fj * dcr_dK1;

    // di'/dK2 = [x_cam / z_cam] * fi * dC(R)/dK2
    derivs[8] = (x_cam/z_cam) * fi * dcr_dK2;
    // dj'/dK2 = [y_cam / z_cam] * fj * dC(R)/dK2
    derivs[9] = (y_cam/z_cam) * fj * dcr_dK2;

    // di'/dK3 = [x_cam / z_cam] * fi * dC(R)/dK3
//    derivs[12] = (x_cam/z_cam) * fi * dcr_dK3;
    // dj'/dK3 = [y_cam / z_cam] * fj * dC(R)/dK3
//    derivs[13] = (y_cam/z_cam) * fj * dcr_dK3;

    // di'/dK4 = [x_cam / z_cam] * fi * dC(R)/dK4
//    derivs[14] = (x_cam/z_cam) * fi * dcr_dK4;
    // dj'/dK4 = [y_cam / z_cam] * fj * dC(R)/dK4
//    derivs[15] = (y_cam/z_cam) * fj * dcr_dK4;
}

void PinholeCameraWithRadialDistortion::getExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d &r_sez, const Eigen::Matrix3d &r_sez_cam) const {

    // Get the position vector in the camera frame
    Eigen::Vector3d r_cam = r_sez_cam * r_sez;
    double x_cam = r_cam[0];
    double y_cam = r_cam[1];
    double z_cam = r_cam[2];

    // Get the ideal (undistorted) image coordinates
    double i, j;
    PinholeCamera::projectVector(r_cam, i, j);

    // Distance of the undistorted point from the distortion (projection) centre;
    // determines the magnitude of the radial distortion
//    double R = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));
    double R = std::sqrt((x_cam/z_cam)*(x_cam/z_cam) + (y_cam/z_cam)*(y_cam/z_cam));

    // Magnitude of the radial distortion
    double CR = getForwardRadialDistortionFactor(R);

    // Get the partial derivatives of the position vector elements with respect to the quaternion elements
    Eigen::Vector3d dr_cam_dq0;
    Eigen::Vector3d dr_cam_dq1;
    Eigen::Vector3d dr_cam_dq2;
    Eigen::Vector3d dr_cam_dq3;

    CoordinateUtil::getSezToCamPartials(r_sez, r_sez_cam, dr_cam_dq0, dr_cam_dq1, dr_cam_dq2, dr_cam_dq3);

    // Some convenience terms:

    // ... derivative factors ...

    // Z * dX/dq0 - X * dZ/dq0
    double x_q0 = (z_cam * dr_cam_dq0[0] - x_cam * dr_cam_dq0[2]);
    // Z * dY/dq0 - X * dZ/dq0
    double y_q0 = (z_cam * dr_cam_dq0[1] - y_cam * dr_cam_dq0[2]);
    // Z * dX/dq1 - X * dZ/dq1
    double x_q1 = (z_cam * dr_cam_dq1[0] - x_cam * dr_cam_dq1[2]);
    // Z * dY/dq1 - X * dZ/dq1
    double y_q1 = (z_cam * dr_cam_dq1[1] - y_cam * dr_cam_dq1[2]);
    // Z * dX/dq2 - X * dZ/dq2
    double x_q2 = (z_cam * dr_cam_dq2[0] - x_cam * dr_cam_dq2[2]);
    // Z * dY/dq2 - X * dZ/dq2
    double y_q2 = (z_cam * dr_cam_dq2[1] - y_cam * dr_cam_dq2[2]);
    // Z * dX/dq3 - X * dZ/dq3
    double x_q3 = (z_cam * dr_cam_dq3[0] - x_cam * dr_cam_dq3[2]);
    // Z * dY/dq3 - X * dZ/dq3
    double y_q3 = (z_cam * dr_cam_dq3[1] - y_cam * dr_cam_dq3[2]);

    double z_cam2 = z_cam * z_cam;

    // ... normalisation constant ...
//    double a = std::sqrt(fi*fi*x_cam*x_cam + fj*fj*y_cam*y_cam);
    double a = std::sqrt(x_cam*x_cam + y_cam*y_cam);

    // ... modified i,j ...
//    double i_mod = fi*fi*x_cam / z_cam2;
//    double j_mod = fj*fj*y_cam / z_cam2;
    double i_mod = x_cam / z_cam2;
    double j_mod = y_cam / z_cam2;

    // ... derivative of the radial distance from distortion centre wrt the orientation parameters ...
    double dR_dq0 = (1.0/a) * (i_mod * x_q0 + j_mod * y_q0);
    double dR_dq1 = (1.0/a) * (i_mod * x_q1 + j_mod * y_q1);
    double dR_dq2 = (1.0/a) * (i_mod * x_q2 + j_mod * y_q2);
    double dR_dq3 = (1.0/a) * (i_mod * x_q3 + j_mod * y_q3);

    // ... derivative of distortion coefficient ...
//    double dC_dR = K1 + 2*K2*R + 3*K3*R*R + 4*K4*R*R*R;
    double dC_dR = 2*K2*R;

    // Putting it all together:

    // di'/dq0
    derivs[0] = fi * ( (x_cam/z_cam) * dC_dR * dR_dq0 + (CR/z_cam2) * x_q0 );
    // dj'/dq0
    derivs[1] = fj * ( (y_cam/z_cam) * dC_dR * dR_dq0 + (CR/z_cam2) * y_q0 );
    // di'/dq1
    derivs[2] = fi * ( (x_cam/z_cam) * dC_dR * dR_dq1 + (CR/z_cam2) * x_q1 );
    // dj'/dq1
    derivs[3] = fj * ( (y_cam/z_cam) * dC_dR * dR_dq1 + (CR/z_cam2) * y_q1 );
    // di'/dq2
    derivs[4] = fi * ( (x_cam/z_cam) * dC_dR * dR_dq2 + (CR/z_cam2) * x_q2 );
    // dj'/dq2
    derivs[5] = fj * ( (y_cam/z_cam) * dC_dR * dR_dq2 + (CR/z_cam2) * y_q2 );
    // di'/dq3
    derivs[6] = fi * ( (x_cam/z_cam) * dC_dR * dR_dq3 + (CR/z_cam2) * x_q3 );
    // dj'/dq3
    derivs[7] = fj * ( (y_cam/z_cam) * dC_dR * dR_dq3 + (CR/z_cam2) * y_q3 );
}

void PinholeCameraWithRadialDistortion::setParameters(const double *params) {
    fi = params[0];
    fj = params[1];
    pi = params[2];
    pj = params[3];
    K2 = params[4];
    init();
}

Eigen::Vector3d PinholeCameraWithRadialDistortion::deprojectPixel(const double & i, const double & j) const {

    // Remove the radial distortion to get the undistorted pixel coordinates
    double i_ideal, j_ideal;
    getUndistortedPixel(i, j, i_ideal, j_ideal);

    // Use function in superclass to deproject undistorted pixel coordinates
    return PinholeCamera::deprojectPixel(i_ideal, j_ideal);
}

bool PinholeCameraWithRadialDistortion::projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const {

    // Use function in superclass to project vector to undistorted pixel coordinates
    double i_ideal, j_ideal;
    PinholeCamera::projectVector(r_cam, i_ideal, j_ideal);

    // Apply radial distortion
    getDistortedPixel(i_ideal, j_ideal, i, j);

    // Determine visibility

    if(r_cam[2] < 0.0) {
        // Ray is behind the camera
        return false;
    }

    // Radial distance of undistorted point from distortion centre
    double r = std::sqrt((i_ideal-pi)*(i_ideal-pi) + (j_ideal-pj)*(j_ideal-pj));

    if(r > r_max) {
        // Ray is outside the valid range for the radial distortion model, so the distorted
        // image coordinates cannot be trusted
        return false;
    }

    if(i<0 || i>width || j<0 || j>height) {
        // Distorted point is outside the image area
        return false;
    }

    // Visibility checks passed
    return true;
}

std::string PinholeCameraWithRadialDistortion::getModelName() const {
    return "PinholeCameraWithRadialDistortion";
}

double PinholeCameraWithRadialDistortion::getForwardRadialDistortionFactor(const double &R) const {
    return 1 + K2*R*R;
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

        // Computes 1 + K2*R^2 + ...
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

    double R = std::sqrt(((i-pi)/fi)*((i-pi)/fi) + ((j-pj)/fj)*((j-pj)/fj));

    // Computes 1 + K2*R^2 + ...
    double CR = getForwardRadialDistortionFactor(R);

    ip = (i - pi)*CR + pi;
    jp = (j - pj)*CR + pj;
}

void PinholeCameraWithRadialDistortion::getUndistortedPixel(const double &ip, const double &jp, double &i, double &j) const {
    double rp = std::sqrt(((ip-pi)/fi)*((ip-pi)/fi) + ((jp-pj)/fj)*((jp-pj)/fj));

    double Drp = getBackwardRadialDistortionFactor(rp, 0.01);

    i = (ip - pi)*Drp + pi;
    j = (jp - pj)*Drp + pj;
}
