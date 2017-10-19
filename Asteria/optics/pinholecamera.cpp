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

double * PinholeCamera::getParameters(unsigned int & n) {

}

void PinholeCamera::setParameters(double * params) {

}

Eigen::Vector3d PinholeCamera::deprojectPixel(const double & i, const double & j) const {
    // Homogenous vector of the image plane coordinates
    Eigen::Vector3d r_im(i, j, 1.0);

    // Deproject to get the unit vector in the camera 3D frame
    Eigen::Vector3d r_cam = kInv * r_im;

    return r_cam;
}

void PinholeCamera::projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const {
    // Project into image coordinates
    Eigen::Vector3d r_im = k * r_cam;
    i = r_im[0] / r_im[2];
    j = r_im[1] / r_im[2];
}
