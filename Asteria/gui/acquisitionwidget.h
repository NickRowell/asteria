#ifndef ACQUISITIONWIDGET_H
#define ACQUISITIONWIDGET_H

#include "infra/acquisitionthread.h"
#include "infra/acquisitionvideostats.h"

#include <QWidget>

class AsteriaState;
class GLMeteorDrawer;
class QPushButton;
class QLabel;
class QCheckBox;

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

    QCheckBox *overlaycheckbox;

    QLabel *acqStateField;
    QLabel *utcField;
    QLabel *fpsField;
    QLabel *totalFramesField;
    QLabel *droppedFramesField;

signals:
    // Forward the signals from the AcquisitionThread
    void acquiredClip(std::string utc);
    void acquiredCalibration(std::string utc);

public slots:
    void updateAcquisitionState(AcquisitionThread::AcquisitionState acqState);
    void updateVideoStats(const AcquisitionVideoStats &);


};

#endif // ACQUISITIONWIDGET_H
