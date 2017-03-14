#ifndef CAMERATAB_H
#define CAMERATAB_H

#include <QWidget>

class MeteorCaptureState;

class CameraTab : public QWidget
{
    Q_OBJECT
public:
    explicit CameraTab(MeteorCaptureState * state = 0, QWidget *parent = 0);

signals:

public slots:

private:
    MeteorCaptureState *state;
};

#endif // CAMERATAB_H
