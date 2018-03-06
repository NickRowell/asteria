#ifndef PINHOLECAMERA_H
#define PINHOLECAMERA_H

#include "optics/cameramodelbase.h"

class PinholeCamera : public CameraModelBase {

public:

    /**
     * @brief Default constructor for the PinholeCamera.
     */
    PinholeCamera();

    /**
     * @brief Main constructor for the PinholeCamera.
     * @param width
     *  Width of the detector [pixels]
     * @param height
     *  Height of the detector [pixels]
     * @param fi
     *  Focal length in the i (horizontal) direction [pixels]
     * @param fj
     *  Focal length in the j (vertical) direction [pixels]
     * @param pi
     *  Coordinate of the principal point in the i (horizontal) direction [pixels]
     * @param pj
     *  Coordinate of the principal point in the j (vertical) direction [pixels]
     */
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

    PinholeCamera * convertToPinholeCamera() const;

    PinholeCameraWithRadialDistortion * convertToPinholeCameraWithRadialDistortion() const;

    PinholeCameraWithSipDistortion * convertToPinholeCameraWithSipDistortion() const;

    unsigned int getNumParameters() const;

    void getParameters(double * params) const;

    void getIntrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_cam) const;

    void getExtrinsicPartialDerivatives(double *derivs, const Eigen::Vector3d & r_sez, const Eigen::Quaterniond & q_sez_cam) const;

    void setParameters(const double *);

    Eigen::Vector3d deprojectPixel(const double & i, const double & j) const;

    bool projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const;

    void getPrincipalPoint(double &pi, double &pj) const;

    void zoom(double &factor);

    void init();

    std::string getModelName() const;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CameraModelBase);
        ar & BOOST_SERIALIZATION_NVP(fi);
        ar & BOOST_SERIALIZATION_NVP(fj);
        ar & BOOST_SERIALIZATION_NVP(pi);
        ar & BOOST_SERIALIZATION_NVP(pj);
        this->init();
    }

};

#endif // PINHOLECAMERA_H
