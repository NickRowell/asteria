#ifndef ANALYSISWORKER_H
#define ANALYSISWORKER_H

#include "infra/meteorcapturestate.h"
#include "infra/image.h"

#include <linux/videodev2.h>
#include <vector>               // vector
#include <memory>               // shared_ptr

#include <QObject>
#include <QMutex>

class AnalysisWorker : public QObject
{
    Q_OBJECT

public:
    AnalysisWorker(QObject *parent = 0, MeteorCaptureState * state = 0, std::vector<std::shared_ptr<Image>> eventFrames = std::vector<std::shared_ptr<Image>>());
    ~AnalysisWorker();

public slots:
    // The command to start processing the images
    void process();

signals:
    // Emitted once processing is complete
    void finished();

private:

    MeteorCaptureState * state;

    std::vector<std::shared_ptr<Image>> eventFrames;
};

#endif // ANALYSISWORKER_H
