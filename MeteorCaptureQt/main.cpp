#include "gui/cameraselectionwindow.h"
#include "gui/configcreationwindow.h"
#include "gui/mainwindow.h"
#include "infra/meteorcapturestate.h"
#include "util/timeutil.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <QDebug>

#include <getopt.h>



using namespace std;

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    qRegisterMetaType<std::shared_ptr<Image>>("std::shared_ptr<Image>");

    // Get the time difference between time of day and the frame timestamp. This needs
    // to be recomputed whenever the computer hibernates.
    long long epochTimeDiffUs = TimeUtil::getEpochTimeShift();

    // Initialise the state object
    MeteorCaptureState * state = new MeteorCaptureState();

    state->epochTimeDiffUs = epochTimeDiffUs;

    // Parse application parameters from the command line

    static struct option long_options[] =
    {
          /* These options set a flag. */
          {"headless", no_argument, &state->headless, 1},
          {"gui", no_argument, &state->headless, 0},
          /* These options don’t set a flag.  We distinguish them by their indices. */
          {"add",     no_argument,       0, 'a'},
          {"delete",  required_argument, 0, 'd'},
          {0, 0, 0, 0}
    };

    /* getopt_long stores the option index here. */
    int option_index = 0;

    int c;
    // The colon after d indicates that an argument follows
    while ((c = getopt_long (argc, argv, "ad:", long_options, &option_index)) != -1) {

        switch (c) {
            case 0: {
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                  break;
                qInfo() << "Option: " << long_options[option_index].name;
                if (optarg) {
                    qInfo() << "Arg: " << optarg;
                }
                break;
            }
            case 'a': {
                qInfo() << "Option a";
                break;
            }
            case 'd': {
                qInfo() << "Option d with value " << optarg;
                break;
            }
            case '?': {
                // getopt_long already printed an option
                break;
            }
            default: {
                abort ();
            }
        }
    }

    qInfo() << "State->headless = " << state->headless;


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
