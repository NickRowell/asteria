#include <QApplication>
#include <QSurfaceFormat>

#include "gui/cameraselectionwindow.h"
#include "gui/configcreationwindow.h"
#include "gui/mainwindow.h"
#include "infra/meteorcapturestate.h"
#include "util/V4L2Util.h"

using namespace std;

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    qRegisterMetaType<std::shared_ptr<Image>>("std::shared_ptr<Image>");

    // TODO: parse application parameters from the command line


    // Get the time difference between time of day and the frame timestamp. This needs
    // to be recomputed whenever the computer hibernates.
    long long epochTimeDiffUs = V4L2Util::getEpochTimeShift();

    MeteorCaptureState * state = new MeteorCaptureState();

    state->epochTimeDiffUs = epochTimeDiffUs;

    CameraSelectionWindow camWin(0, state);

    ConfigCreationWindow configWin(0, state);

    MainWindow mainWin(0, state);


    // Implements advancing from camera window to config window
    QObject::connect(&camWin, SIGNAL (cameraSelected(string)), &configWin, SLOT (show()));
    // Implements returning from config window to camera window
    QObject::connect(&configWin, SIGNAL (cancel()), &camWin, SLOT (show()));
    // Implements advancing from config window to main window
    QObject::connect(&configWin, SIGNAL (ok()), &mainWin, SLOT (slotInit()));

    camWin.show();

    return app.exec();
}
