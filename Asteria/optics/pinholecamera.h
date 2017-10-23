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
     * @brief Pinhole camera matrix.
     * This has the form:
     * \f[
     * K =
     * \left[
     * \begin{array}{rrr}
     * fi &  0 & pi \\
     *  0 & fj & pj \\
     *  0 &  0 &  1
     * \end{array}
     * \right]
     * \f]
     * where fi, fj are the focal lengths and (pi,pj) is the coordinate of
     * the principal point. Note that a non-zero skew parameter (row 0, col 1)
     * is not supported.
     */
    Eigen::Matrix3d k;

    /**
     * @brief Inverse of the pinhole camera matrix.
     */
    Eigen::Matrix3d kInv;

    unsigned int getNumParameters() const;

    void getParameters(double * params) const;

    void getIntrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_cam) const;

    void getExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_sez, const Eigen::Matrix3d & r_sez_cam) const;

    void setParameters(const double *);

    Eigen::Vector3d deprojectPixel(const double & i, const double & j) const;

    void projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const;

};

#endif // PINHOLECAMERA_H
