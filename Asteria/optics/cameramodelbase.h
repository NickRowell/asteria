#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include <Eigen/Dense>


// TODO: implement the get/set parameters methods
// TODO: add methods to get partial derivatives of image coordinates wrt the parameters
// TODO: Add functions to read/write the fields of eahc model to/from file

/**
 * The GeometricOpticsModel class provides a base for all models of the camera
 * projective geometry. The main operations required are projecting vectors expressed
 * in the 3D camera frame to a point in the image, and deprojecting points in the
 * image to a 3D vector in the camera frame.
 */
class CameraModelBase {

public:

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

    // Use functions like this to load/save read/write models to file:
//    friend std::ostream& operator<<(std::ostream& os, const CameraModel& cam)
//    {
//        os << "\nDetector geometry:\n"
//           << "\nPinhole camera matrix:\n" << cam.K
//           << "\nInverse pinhole camera matrix:\n" << cam.invK
//           << "\nImage dimensions: " << cam.width << " " << cam.height
//           << "\nRadial distortion coefficients:"
//           << "\nK0 = " << cam.K0
//           << "\nK1 = " << cam.K1
//           << "\nK2 = " << cam.K2
//           << "\nK3 = " << cam.K3
//           << "\nK4 = " << cam.K4
//           << "\nExposure time [s] = " << cam.exposure_time
//           << "\n\n";

//        // Return a reference to the original ostream so that we can chain
//        // stream insertion operations.
//        return os;
//    }

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
     * @brief Get the partial derivatives of the i coordinate with respect to each of
     * the free parameters.
     * @param derivs
     *  Pointer to the start of the array of double values that on exit will contain the partial
     * derivatives of the parameters of the i coordinate with respect to the parameters
     * of the geometric optics model. The number of elements can be found from getNumParameters().
     * @param r_cam
     *  Camera frame position vector of the point being projected into the image.
     */
    virtual void getPartialDerivativesI(double * derivs, const Eigen::Vector3d & r_cam) const =0;

    /**
     * @brief Get the partial derivatives of the j coordinate with respect to each of
     * the free parameters.
     * @param derivs
     *  Pointer to the start of the array of double values that on exit will contain the partial
     * derivatives of the parameters of the j coordinate with respect to the parameters
     * of the geometric optics model. The number of elements can be found from getNumParameters().
     * @param r_cam
     *  Camera frame position vector of the point being projected into the image.
     */
    virtual void getPartialDerivativesJ(double * derivs, const Eigen::Vector3d & r_cam) const =0;

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

};

#endif /* CAMERAMODEL_H */
