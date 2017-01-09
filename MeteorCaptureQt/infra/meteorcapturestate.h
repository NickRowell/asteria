#ifndef METEORCAPTURESTATE_H
#define METEORCAPTURESTATE_H


class QCameraInfo;

class MeteorCaptureState
{

public:
    // Cannot be loaded from config file: must be created programmatically,
    // either by user selection or automated selection of default camera.
    QCameraInfo * qCameraInfo;




    MeteorCaptureState() {}
};

#endif // METEORCAPTURESTATE_H
