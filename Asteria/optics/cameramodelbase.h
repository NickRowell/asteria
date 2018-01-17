#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include "util/serializationutil.h"

#include <Eigen/Dense>

class PinholeCamera;
class PinholeCameraWithRadialDistortion;

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
     * @brief Converts the camera model to the equivalent PinholeCamera type, or as close as possible
     * given the limitations of the model.
     * @return
     *  A pointer to an equivalent PinholeCamera type.
     */
    virtual PinholeCamera * convertToPinholeCamera() const =0;

    /**
     * @brief Converts the camera model to the equivalent PinholeCameraWithRadialDistortion type, or as close as possible
     * given the limitations of the model.
     * @return
     *  A pointer to an equivalent PinholeCameraWithRadialDistortion type.
     */
    virtual PinholeCameraWithRadialDistortion * convertToPinholeCameraWithRadialDistortion() const =0;

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
     *  Position vector of the point in the CAM frame.
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
     *  Position vector of the point in the SEZ frame.
     * @param q_sez_cam
     *  The unit quaternion that rotates vectors from the SEZ frame to the CAM frame.
     */
    virtual void getExtrinsicPartialDerivatives(double * derivs, const Eigen::Vector3d & r_sez, const Eigen::Quaterniond & q_sez_cam) const =0;

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
     * @return
     *  A boolean value stating whether the projected point is within the visible image area or not. It is important
     * to check this value in order to determine if a projected point is visible or not, and not to simply rely on
     * the pixel coordinates of the projected point. There are a couple of situations where the returned
     * coordinates are inside the image area but the point is not visible:
     * -# The vector originates behind the camera, and within the (reflected) field of view. In this case the projection
     *    equation gives coordinates within the image area but the point is not visible.
     * -# The camera includes distortion terms which are valid only within the image area. In this case, rays
     * originating far outside the field of view can be subject to extreme (and invalid) distortion which incorrectly
     * warps them to coordinates within the image area. This can be detected during the projection.
	 */
    virtual bool projectVector(const Eigen::Vector3d & r_cam, double & i, double & j) const =0;

    /**
     * @brief Get the principal point of the camera, i.e. the point where the camera boresight intersects
     * the image, also the projection of the camera centre.
     * @param pi
     *  The i coordinate of the principal point [pixels]
     * @param pj
     *  The j coordinate of the principal point [pixels]
     */
    virtual void getPrincipalPoint(double &pi, double &pj) const =0;

    /**
     * @brief Apply a scale factor to the focal length etc as appropriate for the camera model in order
     * to support 'zooming' in and out. This is useful for user interaction with the camera model.
     * @param
     *  The zoom factor. This is a positive number; 1.0 is no zoom, values greater than one zoom in.
     */
    virtual void zoom(double &factor) =0;

    /**
     * @brief This function must be called whenever the parameters of the camera model are changed,
     * for example during iterative fitting. It has the task of initialising any other members
     * or convenience fields that are used by the class but which are not formally included in the
     * parameters of the camera model.
     */
    virtual void init() =0;

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
