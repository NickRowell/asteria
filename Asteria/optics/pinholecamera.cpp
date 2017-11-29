#include "optics/pinholecamera.h"
#include "util/coordinateutil.h"

BOOST_CLASS_EXPORT(PinholeCamera)

PinholeCamera::PinholeCamera() : CameraModelBase(), fi(0.0), fj(0.0), pi(0.0), pj(0.0) {

}

PinholeCamera::PinholeCamera(const unsigned int &width, const unsigned int &height, const double &fi, const double &fj, const double &pi, const double &pj) :
   CameraModelBase(width, height), fi(fi), fj(fj), pi(pi), pj(pj) {
    init();
}

PinholeCamera::~PinholeCamera() {

}

unsigned int PinholeCamera::getNumParameters() const {
    return 4;
}

void PinholeCamera::getParameters(double *params) const {
    params[0] = fi;
    params[1] = fj;
    params[2] = pi;
    params[3] = pj;
}

void PinholeCamera::setParameters(const double *params) {
    fi = params[0];
    fj = params[1];
    pi = params[2];
    pj = params[3];
}

void PinholeCamera::getIntrinsicPartialDerivatives(double * derivs, const Eigen::Vector3d & r_cam) const {
    // di/dfi
    derivs[0] = r_cam[0] / r_cam[2];
    // dj/dfi
    derivs[1] = 0.0;
    // di/dfj
    derivs[2] = 0.0;
    // dj/dfj
    derivs[3] = r_cam[1] / r_cam[2];
    // di/dpi
    derivs[4] = 1.0;
    // dj/dpi
    derivs[5] = 0.0;
    // di/dpj
    derivs[6] = 0.0;
    // dj/dpj
    derivs[7] = 1.0;
}

void PinholeCamera::getExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d &r_sez, const Eigen::Matrix3d &r_sez_cam) const {

    // Get the position vector in the camera frame
    Eigen::Vector3d r_cam = r_sez_cam * r_sez;
    double x_cam = r_cam[0];
    double y_cam = r_cam[1];
    double z_cam = r_cam[2];

    // Get the partial derivatives of the position vector elements with respect to the quaternion elements
    Eigen::Vector3d dr_cam_dq0;
    Eigen::Vector3d dr_cam_dq1;
    Eigen::Vector3d dr_cam_dq2;
    Eigen::Vector3d dr_cam_dq3;

    CoordinateUtil::getSezToCamPartials(r_sez, r_sez_cam, dr_cam_dq0, dr_cam_dq1, dr_cam_dq2, dr_cam_dq3);

    double z_cam2 = z_cam * z_cam;

    // di/dq0
    derivs[0] = (fi/z_cam2) * (z_cam * dr_cam_dq0[0] - x_cam * dr_cam_dq0[2]);
    // dj/dq0
    derivs[1] = (fj/z_cam2) * (z_cam * dr_cam_dq0[1] - y_cam * dr_cam_dq0[2]);
    // di/dq1
    derivs[2] = (fi/z_cam2) * (z_cam * dr_cam_dq1[0] - x_cam * dr_cam_dq1[2]);
    // dj/dq1
    derivs[3] = (fj/z_cam2) * (z_cam * dr_cam_dq1[1] - y_cam * dr_cam_dq1[2]);
    // di/dq2
    derivs[4] = (fi/z_cam2) * (z_cam * dr_cam_dq2[0] - x_cam * dr_cam_dq2[2]);
    // dj/dq2
    derivs[5] = (fj/z_cam2) * (z_cam * dr_cam_dq2[1] - y_cam * dr_cam_dq2[2]);
    // di/dq3
    derivs[6] = (fi/z_cam2) * (z_cam * dr_cam_dq3[0] - x_cam * dr_cam_dq3[2]);
    // dj/dq3
    derivs[7] = (fj/z_cam2) * (z_cam * dr_cam_dq3[1] - y_cam * dr_cam_dq3[2]);
}


Eigen::Vector3d PinholeCamera::deprojectPixel(const double & i, const double & j) const {
    // Homogenous vector of the image plane coordinates
    Eigen::Vector3d r_im(i, j, 1.0);

    // Deproject to get the unit vector in the camera 3D frame
    Eigen::Vector3d r_cam = kInv * r_im;
    r_cam.normalize();

    return r_cam;
}

void PinholeCamera::projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const {
    // Project into image coordinates
    Eigen::Vector3d r_im = k * r_cam;
    i = r_im[0] / r_im[2];
    j = r_im[1] / r_im[2];
}

void PinholeCamera::getPrincipalPoint(double &pi, double &pj) const {
    pi = this->pi;
    pj = this->pj;
}

void PinholeCamera::zoom(double &factor) {
    fi *= factor;
    fj *= factor;
}

void PinholeCamera::init() {

    // Initialise the camera matrix
    k <<  fi, 0.0,  pi,
         0.0,  fj,  pj,
         0.0, 0.0, 1.0;

    // Initialise the inverse of the camera matrix
    kInv <<  1.0/fi,     0.0, -pi/fi,
                0.0,  1.0/fj, -pj/fj,
                0.0,     0.0,    1.0;
}

std::string PinholeCamera::getModelName() const {
    return "PinholeCamera";
}
