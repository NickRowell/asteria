#ifndef CALIBRATIONINVENTORY_H
#define CALIBRATIONINVENTORY_H

#include "infra/image.h"

#include <dirent.h>
#include <memory>

#include <QObject>

/**
 * Instances of this class are used to encapsulate all the files and any other outputs
 * from the calibration of the camera. It provides methods to load and save these
 * to disk and to delete all data for individual clips.
 *
 * @brief The CalibrationInventory class
 */
class CalibrationInventory : public QObject {

    Q_OBJECT

public:

    CalibrationInventory();

    /**
     * @brief The median (noise-reduced) image.
     */
    std::shared_ptr<Image> medianImage;

    /**
     * @brief The background image.
     */
    std::shared_ptr<Image> backgroundImage;

    /**
     * @brief A vector containing the individual frames used in the calibration, stored in ascending time order.
     */
    std::vector<std::shared_ptr<Image>> calibrationFrames;

    /**
     * @brief The readnoise [adu]
     */
    double readNoise;

    /**
     * @brief The azimuthal angle of the camera frame [radians]
     * TODO: the SEZ-CAM transformation would be better stored as a quaternion.
     * TODO: convert (az,el,roll) -> quaternion elements (q0,q1,q2,q3).
     */
    double azimuth;

    /**
     * @brief The elevation angle of the camera frame [radians]
     * TODO: the SEZ-CAM transformation would be better stored as a quaternion.
     * TODO: convert (az,el,roll) -> quaternion elements (q0,q1,q2,q3).
     */
    double elevation;

    /**
     * @brief The roll angle of the camera frame [radians]
     * TODO: the SEZ-CAM transformation would be better stored as a quaternion.
     * TODO: convert (az,el,roll) -> quaternion elements (q0,q1,q2,q3).
     */
    double roll;

    // Additional parameters:
    // Camera intrinsic matrix elements
    // Camera radial distortion coefficients


public slots:

    static CalibrationInventory * loadFromDir(std::string path);

    void saveToDir(std::string path);

};

#endif // CALIBRATIONINVENTORY_H
