#include <QApplication>

#include "gui/cameraselectionwindow.h"
#include "gui/configselectionwindow.h"
#include "gui/mainwindow.h"
#include "infra/meteorcapturestate.h"

using namespace std;

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    MeteorCaptureState * state = new MeteorCaptureState();


    CameraSelectionWindow camWin(0, state);
    ConfigSelectionWindow configWin(0, state);
    MainWindow mainWin(0, state);


    // Implements advancing from camera window to config window
    QObject::connect(&camWin, SIGNAL (cameraSelected(QCameraInfo &)), &configWin, SLOT (show()));
    // Implements returning from config window to camera window
    QObject::connect(&configWin, SIGNAL (cancel()), &camWin, SLOT (show()));
    // Implements advancing from config window to main window
    QObject::connect(&configWin, SIGNAL (ok()), &mainWin, SLOT (slotInit()));



    camWin.show();





    return app.exec();
}
