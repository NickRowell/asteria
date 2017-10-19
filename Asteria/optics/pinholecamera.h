#ifndef PINHOLECAMERA_H
#define PINHOLECAMERA_H

#include "optics/cameramodelbase.h"

class PinholeCamera : public CameraModelBase {

public:

    PinholeCamera(const unsigned int &width, const unsigned int &height, const double &fi, const double &fj, const double &pi, const double &pj);
    ~PinholeCamera();

    /**
     * @brief Focal length in the i (horizontal) direction [pixels]
     */
    double fi;

    /**
     * @brief Focal length in the j (vertical) direction [pixels]
     */
    double fj;

    /**
     * @brief Coordinate of the principal point in the i (horizontal) direction [pixels]
     */
    double pi;

    /**
     * @brief Coordinate of the principal point in the j (vertical) direction [pixels]
     */
    double pj;

    /**
     * @brief Pinhole camera matrix. This has the form:
     *
     *      fi   0   pi
     * K =   0  fj   pj
     *       0   0    1
     *
     * where fi, fj are the focal lengths and (pi,pj) is the coordinate of
     * the principal point. Note that a non-zero skew parameter (row 0, col 1)
     * is not supported.
     */
    Eigen::Matrix3d k;

    /**
     * @brief Inverse of camera matrix.
     */
    Eigen::Matrix3d kInv;



    double * getParameters(unsigned int &);

    void setParameters(double *);

    Eigen::Vector3d deprojectPixel(const double & i, const double & j) const;

    void projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const;

};

#endif // PINHOLECAMERA_H
