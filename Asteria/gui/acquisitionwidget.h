#ifndef ACQUISITIONWIDGET_H
#define ACQUISITIONWIDGET_H

#include "infra/acquisitionthread.h"

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
    GLMeteorDrawer * live;

    AcquisitionThread * acqThread;

    QPushButton *play_button;
    QPushButton *detect_button;
    QPushButton *pause_button;

    QLabel *acqStateField;

signals:
    // Forward the signal from the AcquisitionThread
    void acquiredClip(std::string utc);

public slots:
    void updateAcquisitionState(AcquisitionThread::AcquisitionState acqState);


};

#endif // ACQUISITIONWIDGET_H
