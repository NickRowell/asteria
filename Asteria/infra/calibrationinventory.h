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

    // TODO: add calibration data
    std::shared_ptr<Image> peakHold;
    std::vector<std::shared_ptr<Image>> eventFrames;

public slots:

    static CalibrationInventory * loadFromDir(std::string path);

    void saveToDir(std::string path);

};

#endif // ANALYSISINVENTORY_H
