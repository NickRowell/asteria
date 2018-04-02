#ifndef ANALYSISWORKER_H
#define ANALYSISWORKER_H

#include "infra/asteriastate.h"
#include "infra/imageuc.h"

#include <linux/videodev2.h>
#include <vector>               // vector
#include <memory>               // shared_ptr

#include <QObject>

class AnalysisWorker : public QObject
{
    Q_OBJECT

public:
    AnalysisWorker(QObject *parent = 0, AsteriaState * state = 0, const std::shared_ptr<CalibrationInventory> calibration = 0,
                   std::vector<std::shared_ptr<Imageuc>> eventFrames = std::vector<std::shared_ptr<Imageuc>>());
    ~AnalysisWorker();

public slots:
    // The command to start processing the images
    void process();

signals:
    // Emitted once processing is complete
    void finished(std::string utc);

private:

    /**
     * @brief Pointer to the state object that contains various parameters of the event analysis algorithms.
     */
    AsteriaState * state;

    /**
     * @brief Camera calibration; may not be set if there is no calibration available.
     */
    const std::shared_ptr<CalibrationInventory> calibration;

    /**
     * @brief The images containing the event to be analysed.
     */
    std::vector<std::shared_ptr<Imageuc>> eventFrames;
};

#endif // ANALYSISWORKER_H
