#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include <Eigen/Dense>

#include "util/serializationutil.h"

/**
 * @brief The CameraModelBase class provides a base for all models of the camera
 * projective geometry. The main operations required are projecting vectors expressed
 * in the 3D camera frame to a point in the image, and deprojecting points in the
 * image to a 3D vector in the camera frame.
 */
class CameraModelBase {

public:

    /**
     * @brief Default constructor for the CameraModelBase.
     */
    CameraModelBase();

    /**
     * @brief Main constructor for the CameraModelBase.
     * @param width
     *  Width of the detector [pixels]
     * @param height
     *  Height of the detector [pixels]
     */
    CameraModelBase(const unsigned int &width, const unsigned int &height);

    ~CameraModelBase();

    /**
     * Width of the detector [pixels]
     */
    unsigned int width;

    /**
     * Height of the detector [pixels]
     */
    unsigned int height;

    /**
     * @brief Get the number of free parameters of the camera geometric optics model.
     * @return
     *  The number of free parameters of the camera geometric optics model.
     */
    virtual unsigned int getNumParameters() const =0;

    /**
     * @brief Get the parameters of the geometric optics model.
     * @param params  Pointer to the start of the array of double values that on exit
     * will contain the parameters of the geometric optics model. The number of elements
     * can be found from getNumParameters().
     */
    virtual void getParameters(double * params) const =0;

    /**
     * @brief Get the partial derivatives of the (i,j) coordinates with respect to each of
     * the intrinsic parameters of the camera model.
     * @param derivs
     *  Pointer to the start of the array of double values that on exit will contain the partial
     * derivatives of the (i,j) coordinates with respect to the intrinsic parameters of the geometric
     * optics model. The number of elements can be found from 2*getNumParameters().
     * @param r_cam
     *  Camera frame position vector of the point being projected into the image.
     */
    virtual void getIntrinsicPartialDerivatives(double * derivs, const Eigen::Vector3d & r_cam) const =0;

    /**
     * @brief Get the partial derivatives of the (i,j) coordinates with respect to each of
     * the extrinsic parameters of the camera model, being the four quaternion elements that
     * specify the orientation of the camera in the external frame.
     * @param derivs
     *  Pointer to the start of the array of double values that on exit will contain the partial
     * derivatives of the (i,j) coordinates with respect to the extrinsic parameters. There are
     * eight elements, being the derivatives of the (i,j) coordinates with respect to the four
     * quaternion elements.
     * @param r_sez
     *  External frame position vector of the point.
     * @param r_sez_cam
     *  The orthonormal matrix that rotates vectors from the external frame to the camera frame.
     */
    virtual void getExtrinsicPartialDerivatives(double * derivs, const Eigen::Vector3d & r_sez, const Eigen::Matrix3d & r_sez_cam) const =0;

    /**
     * @brief Set the parameters of the geometric optics model.
     * @param params Pointer to the start of the array of double values containing
     * the parameters of the geometric optics model.
     */
    virtual void setParameters(const double * params) =0;

	/**
     * @brief Get the unit vector towards a given detector pixel coordinate.
     *
     * @param i
     *  i coordinate of pixel in image [pixels]
     * @param j
     *  j coordinate of pixel in image [pixels]
     * @return
     *  Camera frame unit vector towards the pixel.
	 */
    virtual Eigen::Vector3d deprojectPixel(const double & i, const double & j) const =0;

	/**
     * @brief Project the given camera frame position vector into the image plane.
     *
     * @param r_cam
     *  Camera frame position vector.
     * @param i
     *  On exit, contains the i image coordinates [pixels]
     * @param j
     *  On exit, contains the j image coordinates [pixels]
	 */
    virtual void projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const =0;

    /**
     * @brief Returns the name of the camera model implemented by the derived class.
     * @return
     *  The name of the camera model implemented by the derived class.
     */
    virtual std::string getModelName() const =0;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_NVP(width);
        ar & BOOST_SERIALIZATION_NVP(height);
    }

};

#endif /* CAMERAMODEL_H */
