#ifndef CAMERASELECTIONWINDOW_H
#define CAMERASELECTIONWINDOW_H

#include <vector>     // provides vector
#include <utility>    // provides pair
#include <string>     // provides string

#include <QWidget>

class QButtonGroup;
class QPushButton;
class MeteorCaptureState;
class GLMeteorDrawer;

using namespace std;

class CameraSelectionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CameraSelectionWindow(QWidget *parent = 0, MeteorCaptureState * state = 0);
private:
    MeteorCaptureState * state;
    vector< pair< string, string > > cameras;
    QButtonGroup *group;
    QPushButton *m_button;
signals:
    void cameraSelected(string cameraPath);
public slots:

private slots:
    void slotCameraButtonClicked();
};

#endif // CAMERASELECTIONWINDOW_H
