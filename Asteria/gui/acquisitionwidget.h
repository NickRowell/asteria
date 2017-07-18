#ifndef ACQUISITIONWIDGET_H
#define ACQUISITIONWIDGET_H

#include "infra/acquisitionthread.h"
#include "infra/videostats.h"

#include <QWidget>

class AsteriaState;
class GLMeteorDrawer;
class QPushButton;
class QLabel;

class AcquisitionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AcquisitionWidget(QWidget * parent = 0, AsteriaState * state = 0);
    ~AcquisitionWidget();

    AsteriaState * state;
    GLMeteorDrawer * display;

    AcquisitionThread * acqThread;

    QPushButton *play_button;
    QPushButton *detect_button;
    QPushButton *pause_button;

    QLabel *acqStateField;
    QLabel *utcField;
    QLabel *fpsField;
    QLabel *totalFramesField;
    QLabel *droppedFramesField;

signals:
    // Forward the signal from the AcquisitionThread
    void acquiredClip(std::string utc);

public slots:
    void updateAcquisitionState(AcquisitionThread::AcquisitionState acqState);
    void updateVideoStats(const VideoStats &);


};

#endif // ACQUISITIONWIDGET_H
