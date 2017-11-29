#ifndef GEOCALFITTER_H
#define GEOCALFITTER_H

#include "infra/referencestar.h"
#include "math/levenbergmarquardtsolver.h"
#include "optics/cameramodelbase.h"

// Get this from cameramodelbase.h
//#include <Eigen/Dense>

class GeoCalFitter : public LevenbergMarquardtSolver
{
public:

    GeoCalFitter(CameraModelBase * cam, Eigen::Quaterniond * q_sez_cam, std::vector<std::pair<Source, ReferenceStar>>  * xms, const double &gmst,
                 const double &lon, const double &lat);

    /**
     * @brief Pointer to the camera model that is being fitted; contains initial guess values
     * for the intrinsic parameters of the camera.
     */
    CameraModelBase * cam;

    /**
     * @brief Pointer to the quaternion defining the orientation of the CAM frame with respect to
     * the SEZ frame. The elements of the quaternion are the extrinsic parameters of the camera.
     */
    Eigen::Quaterniond * q_sez_cam;

    /**
     * @brief Pointer to a vector containing Source / ReferenceStar cross-matches. These provide the
     * observations used to optimize the camera intrinsic (projection) and extrinsic (orientation)
     * parameters.
     */
    std::vector<std::pair<Source, ReferenceStar>>  * xms;

    /**
     * @brief Greenwich mean sidereal time of the calibration. This is required to complete the transformation of reference
     * star positions into the camera frame, and is not considered a parameter of the calibration.
     */
    const double gmst;

    /**
     * @brief Longitude of the observing site [radians]. This is required to complete the transformation of reference
     * star positions into the camera frame, and is not considered a parameter of the calibration.
     */
    const double lon;

    /**
     * @brief Latitude of the observing site [radians]. This is required to complete the transformation of reference
     * star positions into the camera frame, and is not considered a parameter of the calibration.
     */
    const double lat;

    void getModel(const double * params, double * model);

    // One or the other of these should be implemented:
    void getJacobian(const double * params, double * jac);
    void finiteDifferencesStepSizePerParam(double *steps);

    /**
     * @brief Perform LM iteration loop until parameters cannot be improved. This overrides the
     * implementation in the base class because we need to renormalise the quaternion elements
     * after each iteration.
     * @param maxIterations  Maximum number of allowed iteration before convergence.
     * @param verbose        Enables verbose logging
     */
    void fit(unsigned int maxIterations, bool verbose);
};

#endif // GEOCALFITTER_H
