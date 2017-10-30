#ifndef CALIBRATIONWORKER_H
#define CALIBRATIONWORKER_H

#include "infra/asteriastate.h"
#include "infra/imageuc.h"

#include <linux/videodev2.h>
#include <vector>               // vector
#include <memory>               // shared_ptr

#include <QObject>

class CalibrationWorker : public QObject
{
    Q_OBJECT

public:
    CalibrationWorker(QObject *parent = 0, AsteriaState * state = 0, std::vector<std::shared_ptr<Imageuc>> calibrationFrames = std::vector<std::shared_ptr<Imageuc>>());
    ~CalibrationWorker();

public slots:
    // The command to start processing the images
    void process();

signals:
    // Emitted once processing is complete
    void finished(std::string utc);

private:

    AsteriaState * state;

    std::vector<std::shared_ptr<Imageuc>> calibrationFrames;
};

#endif // ANALYSISWORKER_H
