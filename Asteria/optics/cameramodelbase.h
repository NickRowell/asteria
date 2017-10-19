#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include <Eigen/Dense>

/**
 * The GeometricOpticsModel class provides a base for all models of the camera
 * projective geometry. The main operations required are projecting vectors expressed
 * in the 3D camera frame to a point in the image, and deprojecting points in the
 * image to a 3D vector in the camera frame.
 */
class CameraModelBase {

public:

    CameraModelBase(const unsigned int &width, const unsigned int &height);
    ~CameraModelBase();

    /**
     * Width and height of the detector [pixels]
     */
    unsigned int width;
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
     * @brief Get the parameters of the geometric optics model.
     * @param n On exit, this contains the number of
     * parameters stored in the array pointed to by the returned value.
     * @return  Pointer to the start of the array of double values containing
     * the parameters of the geometric optics model.
     */
    virtual double * getParameters(unsigned int & n) =0;

    /**
     * @brief Set the parameters of the geometric optics model.
     * @param params Pointer to the start of the array of double values containing
     * the parameters of the geometric optics model.
     */
    virtual void setParameters(double * params) =0;

	/**
     * @brief Get the unit vector towards a given detector pixel coordinate.
     *
     * @param i
     *  i coordinate of pixel in distorted image [pixels]
     * @param j
     *  j coordinate of pixel in distorted image [pixels]
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
