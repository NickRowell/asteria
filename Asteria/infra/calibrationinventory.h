#ifndef CALIBRATIONINVENTORY_H
#define CALIBRATIONINVENTORY_H

#include "infra/imageuc.h"
#include "infra/imaged.h"
#include "infra/source.h"
#include "infra/referencestar.h"
#include "optics/cameramodelbase.h"

#include <memory>

#include <Eigen/Dense>
#include <QObject>

/**
 * @brief The CalibrationInventory class
 * Instances of this class are used to encapsulate all the files and any other outputs
 * from the calibration of the camera. It provides methods to load and save these
 * to disk and to delete all data for individual clips.
 *
 */
class CalibrationInventory : public QObject {

    Q_OBJECT

public:

    CalibrationInventory();
    CalibrationInventory(const std::vector<std::shared_ptr<Imageuc>> &calibrationFrames);

    /**
     * @brief Records the epoch time in microseconds, i.e. the time elapsed since 1970-01-01T00:00:00Z,
     * of the time that the first byte of image data of the first calibration frame was captured.
     */
    long long epochTimeUs;

    /**
     * @brief The median (noise-reduced) image.
     */
    std::shared_ptr<Imaged> signal;

    /**
     * @brief The noise image.
     */
    std::shared_ptr<Imaged> noise;

    /**
     * @brief The background image.
     */
    std::shared_ptr<Imaged> background;

    /**
     * @brief A vector containing the individual frames used in the calibration, stored in ascending time order.
     */
    std::vector<std::shared_ptr<Imageuc>> calibrationFrames;

    /**
     * @brief A vector containing sources (i.e. observed stars) identified in the calibration image(s).
     */
    std::vector<Source> sources;

    /**
     * @brief A vector containing cross-matches between extracted sources (i.e. observed stars) identified in the
     * calibration image(s) and projected reference stars.
     */
    std::vector<std::pair<Source, ReferenceStar>> xms;

    /**
     * @brief The readnoise [adu]
     */
    double readNoiseAdu;

    /**
     * @brief The orientation of the CAM frame with respect to the SEZ frame.
     */
    Eigen::Quaterniond q_sez_cam;

    /**
     * @brief The geometric optics model for the camera.
     */
    CameraModelBase * cam;

public slots:

    static CalibrationInventory * loadFromDir(std::string path);

    void saveToDir(std::string topLevelPath);

    void deleteCalibration();

};

#endif // CALIBRATIONINVENTORY_H
