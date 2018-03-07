#include "optics/pinholecamerawithradialdistortion.h"
#include "optics/pinholecamerawithsipdistortion.h"
#include "util/coordinateutil.h"

BOOST_CLASS_EXPORT(PinholeCameraWithRadialDistortion)

PinholeCameraWithRadialDistortion::PinholeCameraWithRadialDistortion()  :
    PinholeCamera(), k2(0.0) {

}

PinholeCameraWithRadialDistortion::PinholeCameraWithRadialDistortion(const unsigned int &width, const unsigned int &height, const double &fi,
    const double &fj, const double &pi, const double &pj, const double &k1, const double &k2) :
    PinholeCamera(width, height, fi, fj, pi, pj), k1(k1), k2(k2) {
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
                this->width, this->height, this->fi, this->fj, this->pi, this->pj, this->k1, this->k2);

    return cam;
}

PinholeCameraWithSipDistortion * PinholeCameraWithRadialDistortion::convertToPinholeCameraWithSipDistortion() const {

    fprintf(stderr, "Converting a PinholeCameraWithRadialDistortion to a PinholeCameraWithRadialAndTangentialDistortion\n");

    // Set tangential distortion coefficients to zero
    PinholeCameraWithSipDistortion * cam = new PinholeCameraWithSipDistortion(
                this->width, this->height, this->fi, this->fj, this->pi, this->pj, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    return cam;
}

void PinholeCameraWithRadialDistortion::init() {

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

unsigned int PinholeCameraWithRadialDistortion::getNumParameters() const {
    return 6;
}

void PinholeCameraWithRadialDistortion::getParameters(double * params) const {
    params[0] = fi;
    params[1] = fj;
    params[2] = pi;
    params[3] = pj;
    params[4] = k1;
    params[5] = k2;
}

void PinholeCameraWithRadialDistortion::setParameters(const double *params) {
    fi = params[0];
    fj = params[1];
    pi = params[2];
    pj = params[3];
    k1 = params[4];
    k2 = params[5];
    init();
}

void PinholeCameraWithRadialDistortion::getIntrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_cam) const {

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

void PinholeCameraWithRadialDistortion::getExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d &r_sez, const Quaterniond &q_sez_cam) const {

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

Eigen::Vector3d PinholeCameraWithRadialDistortion::deprojectPixel(const double & ip, const double & jp) const {

    // Remove the distortion to get the undistorted pixel coordinates
    double dip, djp;
    getInverseDistortionOffset(ip, jp, dip, djp, 0.0001);

    double i = ip + dip;
    double j = jp + djp;

    // Use function in superclass to deproject undistorted pixel coordinates
    return PinholeCamera::deprojectPixel(i, j);
}

bool PinholeCameraWithRadialDistortion::projectVector(const Eigen::Vector3d & r_cam, double & ip, double & jp) const {

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

std::string PinholeCameraWithRadialDistortion::getModelName() const {
    return "PinholeCameraWithRadialDistortion";
}

void PinholeCameraWithRadialDistortion::getForwardDistortionOffset(const double &i, const double &j, double &di, double &dj) const {

    double r = std::sqrt(((i-pi)/fi)*((i-pi)/fi) + ((j-pj)/fj)*((j-pj)/fj));

    di = (k1 * r + k2 * r * r) * (i - pi);
    dj = (k1 * r + k2 * r * r) * (j - pj);
}

void PinholeCameraWithRadialDistortion::getInverseDistortionOffset(const double &ip, const double &jp, double &dip, double &djp, const double tol) const {

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

void PinholeCameraWithRadialDistortion::getForwardDistortionIntrinsicPartialDerivatives(double * derivs, const Eigen::Vector3d & r_cam) const {

    double x_cam = r_cam[0];
    double y_cam = r_cam[1];
    double z_cam = r_cam[2];

    double r = std::sqrt((x_cam/z_cam)*(x_cam/z_cam) + (y_cam/z_cam)*(y_cam/z_cam));
    double r2 = r*r;

    // dD/dfi
    derivs[0] = (k1 * r + k2 * r2) * (x_cam/z_cam);
    // dE/dfi
    derivs[1] = 0.0;
    // dD/dfj
    derivs[2] = 0.0;
    // dE/dfj
    derivs[3] = (k1 * r + k2 * r2) * (y_cam/z_cam);
    // dD/dpi
    derivs[4] = 0.0;
    // dE/dpi
    derivs[5] = 0.0;
    // dD/dpj
    derivs[6] = 0.0;
    // dE/dpj
    derivs[7] = 0.0;
    // dD/dK1
    derivs[8] = r * fi * (x_cam/z_cam);
    // dE/dK1
    derivs[9] = r * fj * (y_cam/z_cam);
    // dD/dK2
    derivs[10] = r2 * fi * (x_cam/z_cam);
    // dE/dK2
    derivs[11] = r2 * fj * (y_cam/z_cam);

}

void PinholeCameraWithRadialDistortion::getForwardDistortionExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_sez, const Eigen::Quaterniond &q_sez_cam) const {

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

    // ... radial distance of the undistorted point from the distortion (projection) centre ...
    double r = std::sqrt((x_cam/z_cam)*(x_cam/z_cam) + (y_cam/z_cam)*(y_cam/z_cam));

    double x_zz = x_cam / (z_cam * z_cam);
    double y_zz = y_cam / (z_cam * z_cam);

    // ... derivative factors ...

    // d/dq0 (x_cam/z_cam)
    double x_q0 = (dr_cam_dq0[0] / z_cam) - x_zz * dr_cam_dq0[2];
    // d/dq0 (y_cam/z_cam)
    double y_q0 = (dr_cam_dq0[1] / z_cam) - y_zz * dr_cam_dq0[2];
    // d/dq1 (x_cam/z_cam)
    double x_q1 = (dr_cam_dq1[0] / z_cam) - x_zz * dr_cam_dq1[2];
    // d/dq1 (y_cam/z_cam)
    double y_q1 = (dr_cam_dq1[1] / z_cam) - y_zz * dr_cam_dq1[2];
    // d/dq2 (x_cam/z_cam)
    double x_q2 = (dr_cam_dq2[0] / z_cam) - x_zz * dr_cam_dq2[2];
    // d/dq2 (y_cam/z_cam)
    double y_q2 = (dr_cam_dq2[1] / z_cam) - y_zz * dr_cam_dq2[2];
    // d/dq3 (x_cam/z_cam)
    double x_q3 = (dr_cam_dq3[0] / z_cam) - x_zz * dr_cam_dq3[2];
    // d/dq3 (y_cam/z_cam)
    double y_q3 = (dr_cam_dq3[1] / z_cam) - y_zz * dr_cam_dq3[2];

    // ... normalisation constant ...
    double a = std::sqrt(x_cam*x_cam + y_cam*y_cam);

    // ... derivative of the radial distance from distortion centre wrt the orientation parameters ...
    double dR_dq0 = (1.0/a) * (x_cam * x_q0 + y_cam * y_q0);
    double dR_dq1 = (1.0/a) * (x_cam * x_q1 + y_cam * y_q1);
    double dR_dq2 = (1.0/a) * (x_cam * x_q2 + y_cam * y_q2);
    double dR_dq3 = (1.0/a) * (x_cam * x_q3 + y_cam * y_q3);

    // Put it all together:

    // dD/dq0
    derivs[0] = fi * (x_cam / z_cam) * dR_dq0 * (k1 + 2 * k2 * r) + fi * r * x_q0 * (k1 + k2 * r);
    // dE/dq0
    derivs[1] = fj * (y_cam / z_cam) * dR_dq0 * (k1 + 2 * k2 * r) + fj * r * y_q0 * (k1 + k2 * r);
    // dD/dq1
    derivs[2] = fi * (x_cam / z_cam) * dR_dq1 * (k1 + 2 * k2 * r) + fi * r * x_q1 * (k1 + k2 * r);
    // dE/dq1
    derivs[3] = fj * (y_cam / z_cam) * dR_dq1 * (k1 + 2 * k2 * r) + fj * r * y_q1 * (k1 + k2 * r);
    // dD/dq2
    derivs[4] = fi * (x_cam / z_cam) * dR_dq2 * (k1 + 2 * k2 * r) + fi * r * x_q2 * (k1 + k2 * r);
    // dE/dq2
    derivs[5] = fj * (y_cam / z_cam) * dR_dq2 * (k1 + 2 * k2 * r) + fj * r * y_q2 * (k1 + k2 * r);
    // dD/dq3
    derivs[6] = fi * (x_cam / z_cam) * dR_dq3 * (k1 + 2 * k2 * r) + fi * r * x_q3 * (k1 + k2 * r);
    // dE/dq3
    derivs[7] = fj * (y_cam / z_cam) * dR_dq3 * (k1 + 2 * k2 * r) + fj * r * y_q3 * (k1 + k2 * r);
}
