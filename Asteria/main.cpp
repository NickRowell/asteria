#include "gui/cameraselectionwindow.h"
#include "gui/configcreationwindow.h"
#include "gui/mainwindow.h"
#include "infra/asteriastate.h"
#include "util/timeutil.h"
#include "gui/treeitem.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <QDebug>

#include <getopt.h>

using namespace std;

static void usage(FILE *fp, int argc, char **argv);

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    qRegisterMetaType<std::shared_ptr<Image>>("std::shared_ptr<Image>");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<TreeItem>("TreeItem");

    // Get the time difference between time of day and the frame timestamp. This needs
    // to be recomputed whenever the computer hibernates.
    long long epochTimeDiffUs = TimeUtil::getEpochTimeShift();

    // Initialise the state object
    AsteriaState * state = new AsteriaState();

    state->epochTimeDiffUs = epochTimeDiffUs;

    // Parse application parameters from the command line
    static struct option long_options[] =
    {
          {"help",      no_argument,       NULL,             'h'},
          /* These options set a flag. */
          {"headless",  no_argument,       &state->headless,  1},
          {"gui",       no_argument,       &state->headless,  0},
          /* These options don’t set a flag.  We distinguish them by their indices. */
          {"camera",    required_argument, NULL,              'c'},
          {"config",    required_argument, NULL,              'C'},
          {0,           0,                 NULL,               0}
    };

    /* getopt_long stores the option index here. */
    int option_index = 0;

    int c;
    // The colon after the character indicates that an argument follows
    while ((c = getopt_long (argc, argv, "hc:C:", long_options, &option_index)) != -1) {

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
            case 'h': {
                usage(stderr, argc, argv);
                exit(0);
                break;
            }
            case 'c': {
                qInfo() << "Option camera with value " << optarg;
                break;
            }
            case 'C': {
                qInfo() << "Option config with value " << optarg;
                break;
            }
            case '?': {
                // getopt_long already printed an option
                break;
            }
            default: {
                usage(stderr, argc, argv);
                exit(0);
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


static void usage(FILE *fp, int argc, char **argv)
{
        fprintf(fp,
                 "Usage: %s [options]\n\n"
                 "Options:\n"
                 "-h, --help          Print this message\n"
                 "    --headless      Operate in headless (no GUI) mode\n"
                 "    --gui           Operate in GUI mode\n"
                 "-c, --camera PATH   Use the camera located at PATH (e.g. /dev/video0)\n"
                 "-C, --config PATH   Use the asteria.config file located at PATH\n"
                 "",
                 argv[0]);
}
