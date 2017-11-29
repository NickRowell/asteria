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
    CalibrationWorker(QObject *parent = 0, AsteriaState * state = 0, const CalibrationInventory *initial = 0, std::vector<std::shared_ptr<Imageuc>> calibrationFrames = std::vector<std::shared_ptr<Imageuc>>());
    ~CalibrationWorker();

public slots:
    // The command to start processing the images
    void process();

signals:
    // Emitted once processing is complete
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

    std::vector<std::shared_ptr<Imageuc>> calibrationFrames;
};

#endif // ANALYSISWORKER_H
