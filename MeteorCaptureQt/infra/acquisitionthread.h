#ifndef ACQUISITIONTHREAD_H
#define ACQUISITIONTHREAD_H

#include <linux/videodev2.h>
#include <vector>

#include <QThread>
#include <QMutex>
#include "infra/meteorcapturestate.h"

class AcquisitionThread : public QThread
{
    Q_OBJECT

public:
    AcquisitionThread(QObject *parent = 0, MeteorCaptureState * state = 0);
    ~AcquisitionThread();

    void launch();

signals:
//    void acquiredImage(char * imageBufferStart);
    void acquiredImage(std::vector<char> imageBufferStart);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    MeteorCaptureState * state;
    bool abort;

    /**
     * \brief Information about the video buffer(s) in use.
     */
    struct v4l2_buffer * bufferinfo;

    /**
     * \brief The pixel format in use.
     */
    struct v4l2_format * format;

    /**
     * \brief Information about requested & allocated buffers.
     */
    struct v4l2_requestbuffers * bufrequest;

    /**
     * \brief Array of pointers to the start of each image buffer in memory
     */
    char ** buffer_start;


    QMutex mutex;
};

#endif // ACQUISITIONTHREAD_H
