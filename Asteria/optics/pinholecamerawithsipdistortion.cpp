#include "optics/pinholecamerawithsipdistortion.h"
#include "optics/pinholecamerawithradialdistortion.h"
#include "util/coordinateutil.h"

BOOST_CLASS_EXPORT(PinholeCameraWithSipDistortion)

PinholeCameraWithSipDistortion::PinholeCameraWithSipDistortion()  :
    PinholeCamera(), d0(0.0), d1(0.0), d2(0.0), d3(0.0), d4(0.0), d5(0.0), d6(0.0), e0(0.0), e1(0.0), e2(0.0), e3(0.0), e4(0.0), e5(0.0), e6(0.0) {

}

PinholeCameraWithSipDistortion::PinholeCameraWithSipDistortion(const unsigned int &width, const unsigned int &height, const double &fi,
    const double &fj, const double &pi, const double &pj, const double &d0, const double &d1, const double &d2, const double &d3, const double &d4,
    const double &d5, const double &d6, const double &e0, const double &e1, const double &e2, const double &e3, const double &e4, const double &e5, const double &e6) :
    PinholeCamera(width, height, fi, fj, pi, pj), d0(d0), d1(d1), d2(d2), d3(d3), d4(d4), d5(d5), d6(d6), e0(e0), e1(e1), e2(e2), e3(e3), e4(e4), e5(e5), e6(e6) {
    init();
}

PinholeCameraWithSipDistortion::~PinholeCameraWithSipDistortion() {

}

PinholeCamera * PinholeCameraWithSipDistortion::convertToPinholeCamera() const {

    fprintf(stderr, "Converting a PinholeCameraWithSipDistortion to a PinholeCamera\n");

    // Discard the distortion coefficients
    PinholeCamera * cam = new PinholeCamera(this->width, this->height, this->fi, this->fj, this->pi, this->pj);

    return cam;
}

PinholeCameraWithRadialDistortion * PinholeCameraWithSipDistortion::convertToPinholeCameraWithRadialDistortion() const {

    fprintf(stderr, "Converting a PinholeCameraWithSipDistortion to a PinholeCameraWithRadialDistortion\n");

    PinholeCameraWithRadialDistortion * cam = new PinholeCameraWithRadialDistortion(
                this->width, this->height, this->fi, this->fj, this->pi, this->pj, 0.0, 0.0);

    return cam;
}

PinholeCameraWithSipDistortion * PinholeCameraWithSipDistortion::convertToPinholeCameraWithSipDistortion() const {

    fprintf(stderr, "Converting a PinholeCameraWithSipDistortion to a PinholeCameraWithSipDistortion\n");

    PinholeCameraWithSipDistortion * cam = new PinholeCameraWithSipDistortion(
                this->width, this->height, this->fi, this->fj, this->pi, this->pj, d0, d1, d2, d3, d4, d5, d6, e0, e1, e2, e3, e4, e5, e6);

    return cam;
}

void PinholeCameraWithSipDistortion::init() {

    // Call init() of superclass
    PinholeCamera::init();

    // Compute the maximum distance that an undistorted point can lie from the projection centre
    // and still be visible in the image, given the distortion. This is done by looping around the
    // border of the image and computing the distorted location of each edge pixel, and looking
    // for the point furthest from the principal point.

    r_max = 0.0;

    double r, i, j, dip, djp;

    // Loop along top and bottom edges
    for(double ip=0; ip<=(double)width; ip+=1.0) {

        // Remove distortion from point at top
        double jp = 0;
        getInverseDistortionOffset(ip, jp, dip, djp, 0.0001);

        i = ip + dip;
        j = jp + djp;

        r = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));

        if(r > r_max) {
            r_max = r;
        }

        // Remove distortion from point at bottom
        jp = height;
        getInverseDistortionOffset(ip, jp, dip, djp, 0.0001);

        i = ip + dip;
        j = jp + djp;

        r = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));

        if(r > r_max) {
            r_max = r;
        }
    }

    // Loop along left and right edges
    for(double jp=0; jp<=(double)height; jp+=1.0) {

        // Remove distortion from point at top
        double ip = 0;
        getInverseDistortionOffset(ip, jp, dip, djp, 0.0001);

        i = ip + dip;
        j = jp + djp;

        r = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));

        if(r > r_max) {
            r_max = r;
        }

        // Remove distortion from point at bottom
        ip = width;
        getInverseDistortionOffset(ip, jp, dip, djp, 0.0001);

        i = ip + dip;
        j = jp + djp;

        r = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));

        if(r > r_max) {
            r_max = r;
        }
    }
}

unsigned int PinholeCameraWithSipDistortion::getNumParameters() const {
    return 18;
}

void PinholeCameraWithSipDistortion::setParameters(const double *params) {
    fi = params[0];
    fj = params[1];
    pi = params[2];
    pj = params[3];
    d0 = params[4];
    d1 = params[5];
    d2 = params[6];
    d3 = params[7];
    d4 = params[8];
    d5 = params[9];
    d6 = params[10];
    e0 = params[11];
    e1 = params[12];
    e2 = params[13];
    e3 = params[14];
    e4 = params[15];
    e5 = params[16];
    e6 = params[17];
    init();
}

void PinholeCameraWithSipDistortion::getParameters(double * params) const {
    params[0] = fi;
    params[1] = fj;
    params[2] = pi;
    params[3] = pj;
    params[4] = d0;
    params[5] = d1;
    params[6] = d2;
    params[7] = d3;
    params[8] = d4;
    params[9] = d5;
    params[10] = d6;
    params[11] = e0;
    params[12] = e1;
    params[13] = e2;
    params[14] = e3;
    params[15] = e4;
    params[16] = e5;
    params[17] = e6;
}

void PinholeCameraWithSipDistortion::getIntrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_cam) const {

    double x_cam = r_cam[0];
    double y_cam = r_cam[1];
    double z_cam = r_cam[2];

    // Get the ideal (undistorted) image coordinates
    double i, j;
    PinholeCamera::projectVector(r_cam, i, j);

    // The contribution to the partial derivatives from the pinhole camera part of the model

    // di/dfi
    derivs[0] = x_cam/z_cam;
    // dj/dfi
    derivs[1] = 0.0;
    // di/dfj
    derivs[2] = 0.0;
    // dj/dfj
    derivs[3] = y_cam/z_cam;
    // di/dpi
    derivs[4] = 1.0;
    // dj/dpi
    derivs[5] = 0.0;
    // di/dpj
    derivs[6] = 0.0;
    // dj/dpj
    derivs[7] = 1.0;

    // Initialise remaining partial derivatives (those wrt the parameters of the distortion model) to zero
    for(unsigned int k=8; k<2*getNumParameters(); k++) {
        derivs[k] = 0.0;
    }

    // Compute the (additive) contribution from the distortion model
    double dist_partials[2*getNumParameters()];
    getForwardDistortionIntrinsicPartialDerivatives(dist_partials, r_cam);

    // Add the contribution to the partial derivatives from the distortion model
    for(unsigned int k=0; k<2*getNumParameters(); k++) {
        derivs[k] += dist_partials[k];
    }
}

void PinholeCameraWithSipDistortion::getExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d &r_sez, const Quaterniond &q_sez_cam) const {

    // Get the position vector in the camera frame
    Matrix3d r_sez_cam = q_sez_cam.toRotationMatrix();
    Eigen::Vector3d r_cam = r_sez_cam * r_sez;
    double x_cam = r_cam[0];
    double y_cam = r_cam[1];
    double z_cam = r_cam[2];

    // Get the ideal (undistorted) image coordinates
    double i, j;
    PinholeCamera::projectVector(r_cam, i, j);

    // Get the partial derivatives of the position vector elements with respect to the quaternion elements
    Eigen::Vector3d dr_cam_dq0;
    Eigen::Vector3d dr_cam_dq1;
    Eigen::Vector3d dr_cam_dq2;
    Eigen::Vector3d dr_cam_dq3;

    CoordinateUtil::getSezToCamPartials(r_sez, q_sez_cam, dr_cam_dq0, dr_cam_dq1, dr_cam_dq2, dr_cam_dq3);

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

    // Contribution to the partial derivatives from the pinhole camera part of the model

    // di/dq0
    derivs[0] = (fi / z_cam2) * x_q0;
    // dj/dq0
    derivs[1] = (fj / z_cam2) * y_q0;
    // di/dq1
    derivs[2] = (fi / z_cam2) * x_q1;
    // dj/dq1
    derivs[3] = (fj / z_cam2) * y_q1;
    // di/dq2
    derivs[4] = (fi / z_cam2) * x_q2;
    // dj/dq2
    derivs[5] = (fj / z_cam2) * y_q2;
    // di/dq3
    derivs[6] = (fi / z_cam2) * x_q3;
    // dj/dq3
    derivs[7] = (fj / z_cam2) * y_q3;

    // Contribution to the partial derivatives from the distortion component of the model
    double dist_partials[8];
    getForwardDistortionExtrinsicPartialDerivatives(dist_partials, r_sez, q_sez_cam);

    // Add the contribution to the partial derivatives from the distortion model
    for(unsigned int k=0; k<8; k++) {
        derivs[k] += dist_partials[k];
    }
}

Eigen::Vector3d PinholeCameraWithSipDistortion::deprojectPixel(const double & ip, const double & jp) const {

    // Remove the distortion to get the undistorted pixel coordinates
    double dip, djp;
    getInverseDistortionOffset(ip, jp, dip, djp, 0.0001);

    double i = ip + dip;
    double j = jp + djp;

    // Use function in superclass to deproject undistorted pixel coordinates
    return PinholeCamera::deprojectPixel(i, j);
}

bool PinholeCameraWithSipDistortion::projectVector(const Eigen::Vector3d & r_cam, double & ip, double & jp) const {

    // Use function in superclass to project vector to undistorted pixel coordinates
    double i, j, di, dj;
    PinholeCamera::projectVector(r_cam, i, j);

    // Apply distortion
    getForwardDistortionOffset(i, j, di, dj);

    ip = i + di;
    jp = j + dj;

    // Determine visibility

    if(r_cam[2] < 0.0) {
        // Ray is behind the camera
        return false;
    }

    // Radial distance of undistorted point from distortion centre
    double r = std::sqrt((i-pi)*(i-pi) + (j-pj)*(j-pj));

    if(r > r_max) {
        // Ray is outside the valid range for the radial distortion model, so the distorted
        // image coordinates cannot be trusted
        return false;
    }

    if(ip<0 || ip>width || jp<0 || jp>height) {
        // Distorted point is outside the image area
        return false;
    }

    // Visibility checks passed
    return true;
}

std::string PinholeCameraWithSipDistortion::getModelName() const {
    return "PinholeCameraWithSipDistortion";
}

void PinholeCameraWithSipDistortion::getForwardDistortionOffset(const double &i, const double &j, double &di, double &dj) const {

    double ii = i - pi;
    double jj = j - pj;

    di = d0*ii*ii + d1*jj*jj + d2*ii*jj + d3*ii*ii*jj + d4*ii*jj*jj + d5*ii*ii*ii + d6*jj*jj*jj;
    dj = e0*ii*ii + e1*jj*jj + e2*ii*jj + e3*ii*ii*jj + e4*ii*jj*jj + e5*ii*ii*ii + e6*jj*jj*jj;
}

void PinholeCameraWithSipDistortion::getInverseDistortionOffset(const double &ip, const double &jp, double &dip, double &djp, const double tol) const {

    // Current guess for the undistorted pixel coordinates, initialised to the distorted pixel coordinates
    // according to the iterative inversion algorithm
    double i_k = ip;
    double j_k = jp;

    // Updated guess for the undistorted pixel coordinates
    double i_kp1;
    double j_kp1;

    // Iterations limit
    unsigned int MAX_ITERATIONS=1000;

    // Loop until converged
    while(MAX_ITERATIONS-- > 0) {

        // Get the forward distortion offset at the current estimate for the undistorted pixel coordinates (i, j)
        double di_k, dj_k;
        getForwardDistortionOffset(i_k, j_k, di_k, dj_k);

        // Check for convergence: difference between observed point and the distorted ideal point
        double ip_k = i_k + di_k;
        double jp_k = j_k + dj_k;
        double r = std::sqrt((ip_k - ip)*(ip_k - ip) + (jp_k - jp)*(jp_k - jp));
        if(r < tol) {
            // Converged
            break;
        }

        // Update is equal to the difference between the estimated distorted point and the observed distorted point
        i_kp1 = i_k + (ip - ip_k);
        j_kp1 = j_k + (jp - jp_k);

        // If problems with converge emerge in pathological cases, try taking average of i_k and the default update
//        i_kp1 = 0.5 * (i_k + (ip - di_k));
//        j_kp1 = 0.5 * (j_k + (jp - dj_k));

        // Apply iteration
        i_k = i_kp1;
        j_k = j_kp1;
    }

    dip = i_k - ip;
    djp = j_k - jp;
}

void PinholeCameraWithSipDistortion::getForwardDistortionIntrinsicPartialDerivatives(double * derivs, const Eigen::Vector3d & r_cam) const {

    double x_cam = r_cam[0];
    double y_cam = r_cam[1];
    double z_cam = r_cam[2];

    // dD/dfi
    derivs[0] = 2.0*d0*fi*(x_cam/z_cam)*(x_cam/z_cam) + d2*fj*(y_cam/z_cam)*(x_cam/z_cam) + 2.0*d3*fi*fj*(y_cam/z_cam)*(x_cam/z_cam)*(x_cam/z_cam) + d4*fj*fj*(x_cam/z_cam)*(y_cam/z_cam)*(y_cam/z_cam) + 3*d5*fi*fi*(x_cam/z_cam)*(x_cam/z_cam)*(x_cam/z_cam);
    // dE/dfi
    derivs[1] = 2.0*e0*fi*(x_cam/z_cam)*(x_cam/z_cam) + e2*fj*(y_cam/z_cam)*(x_cam/z_cam) + 2.0*e3*fi*fj*(y_cam/z_cam)*(x_cam/z_cam)*(x_cam/z_cam) + e4*fj*fj*(x_cam/z_cam)*(y_cam/z_cam)*(y_cam/z_cam) + 3*e5*fi*fi*(x_cam/z_cam)*(x_cam/z_cam)*(x_cam/z_cam);

    // dD/dfj
    derivs[2] = 2.0*d1*fj*(y_cam/z_cam)*(y_cam/z_cam) + d2*fi*(y_cam/z_cam)*(x_cam/z_cam) + d3*fi*fi*(y_cam/z_cam)*(x_cam/z_cam)*(x_cam/z_cam) + 2.0*d4*fi*fj*(x_cam/z_cam)*(y_cam/z_cam)*(y_cam/z_cam) + 3*d6*fj*fj*(y_cam/z_cam)*(y_cam/z_cam)*(y_cam/z_cam);
    // dE/dfj
    derivs[3] = 2.0*e1*fj*(y_cam/z_cam)*(y_cam/z_cam) + e2*fi*(y_cam/z_cam)*(x_cam/z_cam) + e3*fi*fi*(y_cam/z_cam)*(x_cam/z_cam)*(x_cam/z_cam) + 2.0*e4*fi*fj*(x_cam/z_cam)*(y_cam/z_cam)*(y_cam/z_cam) + 3*e6*fj*fj*(y_cam/z_cam)*(y_cam/z_cam)*(y_cam/z_cam);

    // dD/dpi
    derivs[4] = 0.0;
    // dE/dpi
    derivs[5] = 0.0;
    // dD/dpj
    derivs[6] = 0.0;
    // dE/dpj
    derivs[7] = 0.0;

    double ii = fi * (x_cam/z_cam);
    double jj = fj * (y_cam/z_cam);

    // dD/dd0
    derivs[8] = ii*ii;
    // dE/dd0
    derivs[9] = 0.0;
    // dD/dd1
    derivs[10] = jj*jj;
    // dE/dd1
    derivs[11] = 0.0;
    // dD/dd2
    derivs[12] = ii*jj;
    // dE/dd2
    derivs[13] = 0.0;
    // dD/dd3
    derivs[14] = ii*ii*jj;
    // dE/dd3
    derivs[15] = 0.0;
    // dD/dd4
    derivs[16] = ii*jj*jj;
    // dE/dd4
    derivs[17] = 0.0;
    // dD/dd5
    derivs[18] = ii*ii*ii;
    // dE/dd5
    derivs[19] = 0.0;
    // dD/dd6
    derivs[20] = jj*jj*jj;
    // dE/dd6
    derivs[21] = 0.0;

    // dD/de0
    derivs[22] = 0.0;
    // dE/de0
    derivs[23] = ii*ii;
    // dD/de1
    derivs[24] = 0.0;
    // dE/de1
    derivs[25] = jj*jj;
    // dD/de2
    derivs[26] = 0.0;
    // dE/de2
    derivs[27] = ii*jj;
    // dD/de3
    derivs[28] = 0.0;
    // dE/de3
    derivs[29] = ii*ii*jj;
    // dD/de4
    derivs[30] = 0.0;
    // dE/de4
    derivs[31] = ii*jj*jj;
    // dD/de5
    derivs[32] = 0.0;
    // dE/de5
    derivs[33] = ii*ii*ii;
    // dD/de6
    derivs[34] = 0.0;
    // dE/de6
    derivs[35] = jj*jj*jj;
}

void PinholeCameraWithSipDistortion::getForwardDistortionExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_sez, const Eigen::Quaterniond &q_sez_cam) const {

    // Compute some convenience terms

    // Get the position vector in the camera frame
    Matrix3d r_sez_cam = q_sez_cam.toRotationMatrix();
    Eigen::Vector3d r_cam = r_sez_cam * r_sez;
    double x_cam = r_cam[0];
    double y_cam = r_cam[1];
    double z_cam = r_cam[2];

    // Get the ideal (undistorted) image coordinates
    double i, j;
    PinholeCamera::projectVector(r_cam, i, j);

    // Get the partial derivatives of the position vector elements with respect to the quaternion elements
    Eigen::Vector3d dr_cam_dq0;
    Eigen::Vector3d dr_cam_dq1;
    Eigen::Vector3d dr_cam_dq2;
    Eigen::Vector3d dr_cam_dq3;

    CoordinateUtil::getSezToCamPartials(r_sez, q_sez_cam, dr_cam_dq0, dr_cam_dq1, dr_cam_dq2, dr_cam_dq3);

    // Some convenience terms:

    // ... extract derivative factors ...

    double dx_dq0 = dr_cam_dq0[0];
    double dy_dq0 = dr_cam_dq0[1];
    double dz_dq0 = dr_cam_dq0[2];

    double dx_dq1 = dr_cam_dq1[0];
    double dy_dq1 = dr_cam_dq1[1];
    double dz_dq1 = dr_cam_dq1[2];

    double dx_dq2 = dr_cam_dq2[0];
    double dy_dq2 = dr_cam_dq2[1];
    double dz_dq2 = dr_cam_dq2[2];

    double dx_dq3 = dr_cam_dq3[0];
    double dy_dq3 = dr_cam_dq3[1];
    double dz_dq3 = dr_cam_dq3[2];

    double x_cam2 = x_cam * x_cam;
    double x_cam3 = x_cam2 * x_cam;

    double y_cam2 = y_cam * y_cam;
    double y_cam3 = y_cam2 * y_cam;

    double z_cam2 = z_cam*z_cam;
    double z_cam3 = z_cam2*z_cam;
    double z_cam4 = z_cam3*z_cam;

    // dD/dq0
    derivs[0] = d0 * fi * fi * (2.0 * x_cam * dx_dq0 / z_cam2 - 2.0 * x_cam2 * dz_dq0 / z_cam3) +
                d1 * fj * fj * (2.0 * y_cam * dy_dq0 / z_cam2 - 2.0 * y_cam2 * dz_dq0 / z_cam3) +
                d2 * fi * fj * ((x_cam / z_cam2) * dy_dq0 + (y_cam / z_cam2) * dx_dq0 - (2.0 * x_cam * y_cam / z_cam3) * dz_dq0) +
                d3 * fi * fi * fj * ((x_cam2 / z_cam3) * dy_dq0 + (2.0 * x_cam * y_cam / z_cam3) * dx_dq0 - (3.0 * x_cam2 * y_cam / z_cam4) * dz_dq0) +
                d4 * fi * fj * fj * ((y_cam2 / z_cam3) * dx_dq0 + (2.0 * x_cam * y_cam / z_cam3) * dy_dq0 - (3.0 * x_cam * y_cam2 / z_cam4) * dz_dq0) +
                d5 * fi * fi * fi * ((3.0 * x_cam2 / z_cam3) * dx_dq0 - (3.0 * x_cam3 / z_cam4) * dz_dq0) +
                d6 * fj * fj * fj * ((3.0 * y_cam2 / z_cam3) * dy_dq0 - (3.0 * y_cam3 / z_cam4) * dz_dq0);

    // dE/dq0
    derivs[1] = e0 * fi * fi * (2.0 * x_cam * dx_dq0 / z_cam2 - 2.0 * x_cam2 * dz_dq0 / z_cam3) +
                e1 * fj * fj * (2.0 * y_cam * dy_dq0 / z_cam2 - 2.0 * y_cam2 * dz_dq0 / z_cam3) +
                e2 * fi * fj * ((x_cam / z_cam2) * dy_dq0 + (y_cam / z_cam2) * dx_dq0 - (2.0 * x_cam * y_cam / z_cam3) * dz_dq0) +
                e3 * fi * fi * fj * ((x_cam2 / z_cam3) * dy_dq0 + (2.0 * x_cam * y_cam / z_cam3) * dx_dq0 - (3.0 * x_cam2 * y_cam / z_cam4) * dz_dq0) +
                e4 * fi * fj * fj * ((y_cam2 / z_cam3) * dx_dq0 + (2.0 * x_cam * y_cam / z_cam3) * dy_dq0 - (3.0 * x_cam * y_cam2 / z_cam4) * dz_dq0) +
                e5 * fi * fi * fi * ((3.0 * x_cam2 / z_cam3) * dx_dq0 - (3.0 * x_cam3 / z_cam4) * dz_dq0) +
                e6 * fj * fj * fj * ((3.0 * y_cam2 / z_cam3) * dy_dq0 - (3.0 * y_cam3 / z_cam4) * dz_dq0);

    // dD/dq1
    derivs[2] = d0 * fi * fi * (2.0 * x_cam * dx_dq1 / z_cam2 - 2.0 * x_cam2 * dz_dq1 / z_cam3) +
                d1 * fj * fj * (2.0 * y_cam * dy_dq1 / z_cam2 - 2.0 * y_cam2 * dz_dq1 / z_cam3) +
                d2 * fi * fj * ((x_cam / z_cam2) * dy_dq1 + (y_cam / z_cam2) * dx_dq1 - (2.0 * x_cam * y_cam / z_cam3) * dz_dq1) +
                d3 * fi * fi * fj * ((x_cam2 / z_cam3) * dy_dq1 + (2.0 * x_cam * y_cam / z_cam3) * dx_dq1 - (3.0 * x_cam2 * y_cam / z_cam4) * dz_dq1) +
                d4 * fi * fj * fj * ((y_cam2 / z_cam3) * dx_dq1 + (2.0 * x_cam * y_cam / z_cam3) * dy_dq1 - (3.0 * x_cam * y_cam2 / z_cam4) * dz_dq1) +
                d5 * fi * fi * fi * ((3.0 * x_cam2 / z_cam3) * dx_dq1 - (3.0 * x_cam3 / z_cam4) * dz_dq1) +
                d6 * fj * fj * fj * ((3.0 * y_cam2 / z_cam3) * dy_dq1 - (3.0 * y_cam3 / z_cam4) * dz_dq1);

    // dE/dq1
    derivs[3] = e0 * fi * fi * (2.0 * x_cam * dx_dq1 / z_cam2 - 2.0 * x_cam2 * dz_dq1 / z_cam3) +
                e1 * fj * fj * (2.0 * y_cam * dy_dq1 / z_cam2 - 2.0 * y_cam2 * dz_dq1 / z_cam3) +
                e2 * fi * fj * ((x_cam / z_cam2) * dy_dq1 + (y_cam / z_cam2) * dx_dq1 - (2.0 * x_cam * y_cam / z_cam3) * dz_dq1) +
                e3 * fi * fi * fj * ((x_cam2 / z_cam3) * dy_dq1 + (2.0 * x_cam * y_cam / z_cam3) * dx_dq1 - (3.0 * x_cam2 * y_cam / z_cam4) * dz_dq1) +
                e4 * fi * fj * fj * ((y_cam2 / z_cam3) * dx_dq1 + (2.0 * x_cam * y_cam / z_cam3) * dy_dq1 - (3.0 * x_cam * y_cam2 / z_cam4) * dz_dq1) +
                e5 * fi * fi * fi * ((3.0 * x_cam2 / z_cam3) * dx_dq1 - (3.0 * x_cam3 / z_cam4) * dz_dq1) +
                e6 * fj * fj * fj * ((3.0 * y_cam2 / z_cam3) * dy_dq1 - (3.0 * y_cam3 / z_cam4) * dz_dq1);

    // dD/dq2
    derivs[4] = d0 * fi * fi * (2.0 * x_cam * dx_dq2 / z_cam2 - 2.0 * x_cam2 * dz_dq2 / z_cam3) +
                d1 * fj * fj * (2.0 * y_cam * dy_dq2 / z_cam2 - 2.0 * y_cam2 * dz_dq2 / z_cam3) +
                d2 * fi * fj * ((x_cam / z_cam2) * dy_dq2 + (y_cam / z_cam2) * dx_dq2 - (2.0 * x_cam * y_cam / z_cam3) * dz_dq2) +
                d3 * fi * fi * fj * ((x_cam2 / z_cam3) * dy_dq2 + (2.0 * x_cam * y_cam / z_cam3) * dx_dq2 - (3.0 * x_cam2 * y_cam / z_cam4) * dz_dq2) +
                d4 * fi * fj * fj * ((y_cam2 / z_cam3) * dx_dq2 + (2.0 * x_cam * y_cam / z_cam3) * dy_dq2 - (3.0 * x_cam * y_cam2 / z_cam4) * dz_dq2) +
                d5 * fi * fi * fi * ((3.0 * x_cam2 / z_cam3) * dx_dq2 - (3.0 * x_cam3 / z_cam4) * dz_dq2) +
                d6 * fj * fj * fj * ((3.0 * y_cam2 / z_cam3) * dy_dq2 - (3.0 * y_cam3 / z_cam4) * dz_dq2);

    // dE/dq2
    derivs[5] = e0 * fi * fi * (2.0 * x_cam * dx_dq2 / z_cam2 - 2.0 * x_cam2 * dz_dq2 / z_cam3) +
                e1 * fj * fj * (2.0 * y_cam * dy_dq2 / z_cam2 - 2.0 * y_cam2 * dz_dq2 / z_cam3) +
                e2 * fi * fj * ((x_cam / z_cam2) * dy_dq2 + (y_cam / z_cam2) * dx_dq2 - (2.0 * x_cam * y_cam / z_cam3) * dz_dq2) +
                e3 * fi * fi * fj * ((x_cam2 / z_cam3) * dy_dq2 + (2.0 * x_cam * y_cam / z_cam3) * dx_dq2 - (3.0 * x_cam2 * y_cam / z_cam4) * dz_dq2) +
                e4 * fi * fj * fj * ((y_cam2 / z_cam3) * dx_dq2 + (2.0 * x_cam * y_cam / z_cam3) * dy_dq2 - (3.0 * x_cam * y_cam2 / z_cam4) * dz_dq2) +
                e5 * fi * fi * fi * ((3.0 * x_cam2 / z_cam3) * dx_dq2 - (3.0 * x_cam3 / z_cam4) * dz_dq2) +
                e6 * fj * fj * fj * ((3.0 * y_cam2 / z_cam3) * dy_dq2 - (3.0 * y_cam3 / z_cam4) * dz_dq2);

    // dD/dq3
    derivs[6] = d0 * fi * fi * (2.0 * x_cam * dx_dq3 / z_cam2 - 2.0 * x_cam2 * dz_dq3 / z_cam3) +
                d1 * fj * fj * (2.0 * y_cam * dy_dq3 / z_cam2 - 2.0 * y_cam2 * dz_dq3 / z_cam3) +
                d2 * fi * fj * ((x_cam / z_cam2) * dy_dq3 + (y_cam / z_cam2) * dx_dq3 - (2.0 * x_cam * y_cam / z_cam3) * dz_dq3) +
                d3 * fi * fi * fj * ((x_cam2 / z_cam3) * dy_dq3 + (2.0 * x_cam * y_cam / z_cam3) * dx_dq3 - (3.0 * x_cam2 * y_cam / z_cam4) * dz_dq3) +
                d4 * fi * fj * fj * ((y_cam2 / z_cam3) * dx_dq3 + (2.0 * x_cam * y_cam / z_cam3) * dy_dq3 - (3.0 * x_cam * y_cam2 / z_cam4) * dz_dq3) +
                d5 * fi * fi * fi * ((3.0 * x_cam2 / z_cam3) * dx_dq3 - (3.0 * x_cam3 / z_cam4) * dz_dq3) +
                d6 * fj * fj * fj * ((3.0 * y_cam2 / z_cam3) * dy_dq3 - (3.0 * y_cam3 / z_cam4) * dz_dq3);

    // dE/dq3
    derivs[7] = e0 * fi * fi * (2.0 * x_cam * dx_dq3 / z_cam2 - 2.0 * x_cam2 * dz_dq3 / z_cam3) +
                e1 * fj * fj * (2.0 * y_cam * dy_dq3 / z_cam2 - 2.0 * y_cam2 * dz_dq3 / z_cam3) +
                e2 * fi * fj * ((x_cam / z_cam2) * dy_dq3 + (y_cam / z_cam2) * dx_dq3 - (2.0 * x_cam * y_cam / z_cam3) * dz_dq3) +
                e3 * fi * fi * fj * ((x_cam2 / z_cam3) * dy_dq3 + (2.0 * x_cam * y_cam / z_cam3) * dx_dq3 - (3.0 * x_cam2 * y_cam / z_cam4) * dz_dq3) +
                e4 * fi * fj * fj * ((y_cam2 / z_cam3) * dx_dq3 + (2.0 * x_cam * y_cam / z_cam3) * dy_dq3 - (3.0 * x_cam * y_cam2 / z_cam4) * dz_dq3) +
                e5 * fi * fi * fi * ((3.0 * x_cam2 / z_cam3) * dx_dq3 - (3.0 * x_cam3 / z_cam4) * dz_dq3) +
                e6 * fj * fj * fj * ((3.0 * y_cam2 / z_cam3) * dy_dq3 - (3.0 * y_cam3 / z_cam4) * dz_dq3);
}
