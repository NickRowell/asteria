#ifndef CAMERASELECTIONWINDOW_H
#define CAMERASELECTIONWINDOW_H

#include <QWidget>
#include <QCameraInfo>

class QButtonGroup;
class QPushButton;
class MeteorCaptureState;

class CameraSelectionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CameraSelectionWindow(QWidget *parent = 0, MeteorCaptureState * state = 0);
private:
    MeteorCaptureState * state;
    QList<QCameraInfo> cameras;
    QButtonGroup *group;
    QPushButton *m_button;
signals:
    void cameraSelected(QCameraInfo & camInfo);
public slots:

private slots:
    void slotCameraButtonClicked();
};

#endif // CAMERASELECTIONWINDOW_H
