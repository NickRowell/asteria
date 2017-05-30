#include "gui/cameraselectionwindow.h"
#include "gui/configcreationwindow.h"
#include "gui/mainwindow.h"
#include "infra/asteriastate.h"
#include "util/timeutil.h"

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

    // Get the time difference between time of day and the frame timestamp. This needs
    // to be recomputed whenever the computer hibernates.
    long long epochTimeDiffUs = TimeUtil::getEpochTimeShift();

    // Initialise the state object
    AsteriaState * state = new AsteriaState();

    state->epochTimeDiffUs = epochTimeDiffUs;

    // Parse application parameters from the command line
    static struct option long_options[] =
    {
          /* These options set a flag. */
          {"headless",  no_argument,       &state->headless,  1},
          {"gui",       no_argument,       &state->headless,  0},
          /* These options don’t set a flag.  We distinguish them by their indices. */
          {"add",       no_argument,       NULL,              'a'},
          {"delete",    required_argument, NULL,              'd'},
          {0,           0,                 NULL,               0}
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
                usage(stderr, argc, argv);
                abort ();
            }
        }
    }

    qInfo() << "State->headless = " << state->headless;

    usage(stderr, argc, argv);
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
                 "Version 1.3\n"
                 "Options:\n"
                 "-h | --help          Print this message\n"
                 "-m | --mmap          Use memory mapped buffers [default]\n"
                 "-r | --read          Use read() calls\n"
                 "-u | --userp         Use application allocated buffers\n"
                 "-o | --output        Outputs stream to stdout\n"
                 "-f | --format        Force format to 640x480 YUYV\n"
                 "",
                 argv[0]);
}
