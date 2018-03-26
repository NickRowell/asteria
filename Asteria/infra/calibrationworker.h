#ifndef CALIBRATIONWORKER_H
#define CALIBRATIONWORKER_H

#include "infra/asteriastate.h"
#include "infra/imageuc.h"
#include "infra/calibrationinventory.h"

#include <linux/videodev2.h>
#include <vector>               // vector
#include <memory>               // shared_ptr

#include <QObject>

class CalibrationWorker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for the CalibrationWorker.
     * @param parent
     *  The parent widget, if it exists.
     * @param state
     *  Pointer to the AsteriaState object that contains various parameters of the calibration algorithms.
     * @param initial
     *  Pointer to the initial CalibrationInventory which will provide initial guess solution and be used to
     * propagate certain calibrations in time.
     * @param calibrationFrames
     *  Vector of frames to be used to determine calibration.
     */
    CalibrationWorker(QObject *parent = 0, AsteriaState * state = 0, const CalibrationInventory *initial = 0,
                      std::vector<std::shared_ptr<Imageuc>> calibrationFrames = std::vector<std::shared_ptr<Imageuc>>());
    ~CalibrationWorker();

public slots:

    /**
     * @brief The command to start processing the calibration.
     */
    void process();

signals:

    /**
     * @brief Emitted once processing is complete.
     * @param utc
     *  String containing the UTC of the new calibration; the calibration data is stored in a directory with this name.
     */
    void finished(std::string utc);

private:

    /**
     * @brief Pointer to the state object that contains various parameters of the calibration algorithms.
     */
    AsteriaState * state;

    /**
     * @brief Initial guess calibration, used to initialise the fit and propagate certain calibrations in time.
     */
    const CalibrationInventory * initial;

    /**
     * @brief Vector of frames to be used to determine calibration.
     */
    std::vector<std::shared_ptr<Imageuc>> calibrationFrames;
};

#endif // ANALYSISWORKER_H
