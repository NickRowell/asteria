#ifndef ACQUISITIONWIDGET_H
#define ACQUISITIONWIDGET_H

#include "infra/acquisitionthread.h"

#include <QWidget>

class AsteriaState;
class GLMeteorDrawer;

class AcquisitionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AcquisitionWidget(QWidget * parent = 0, AsteriaState * state = 0);
    ~AcquisitionWidget();

    AsteriaState * state;
    GLMeteorDrawer * live;

    AcquisitionThread * acqThread;

signals:
    // Forward the signal from the AcquisitionThread
    void acquiredClip(std::string utc);

public slots:


};

#endif // ACQUISITIONWIDGET_H
