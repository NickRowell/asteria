#include "pinholecamera.h"

PinholeCamera::PinholeCamera(const unsigned int &width, const unsigned int &height, const double &fi, const double &fj, const double &pi, const double &pj) :
   CameraModelBase(width, height), fi(fi), fj(fj), pi(pi), pj(pj) {

    // Initialise the camera matrix
    k <<  fi, 0.0,  pi,
         0.0,  fj,  pj,
         0.0, 0.0, 1.0;

    // Initialise the inverse of the camera matrix
    kInv <<  1.0/fi,     0.0, -pi/fi,
                0.0,  1.0/fj, -pj/fj,
                0.0,     0.0,    1.0;
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

void PinholeCamera::getPartialDerivativesI(double * derivs, const Eigen::Vector3d & r_cam) const {
    // di/dfi
    derivs[0] = r_cam[0] / r_cam[2];
    // di/dfj
    derivs[1] = 0.0;
    // di/dpi
    derivs[2] = 1.0;
    // di/dpj
    derivs[3] = 0.0;
}

void PinholeCamera::getPartialDerivativesJ(double *derivs, const Eigen::Vector3d &r_cam) const {
    // dj/dfi
    derivs[0] = 0.0;
    // dj/dfj
    derivs[1] = r_cam[1] / r_cam[2];
    // dj/dpi
    derivs[2] = 0.0;
    // dj/dpj
    derivs[3] = 1.0;
}

void PinholeCamera::setParameters(const double *params) {
    fi = params[0];
    fj = params[1];
    pi = params[2];
    pj = params[3];
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
