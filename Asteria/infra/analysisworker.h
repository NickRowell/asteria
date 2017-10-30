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
    AnalysisWorker(QObject *parent = 0, AsteriaState * state = 0, std::vector<std::shared_ptr<Imageuc>> eventFrames = std::vector<std::shared_ptr<Imageuc>>());
    ~AnalysisWorker();

public slots:
    // The command to start processing the images
    void process();

signals:
    // Emitted once processing is complete
    void finished(std::string utc);

private:

    AsteriaState * state;

    std::vector<std::shared_ptr<Imageuc>> eventFrames;
};

#endif // ANALYSISWORKER_H
