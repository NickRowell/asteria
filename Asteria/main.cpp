#include "gui/cameraselectionwindow.h"
#include "gui/configcreationwindow.h"
#include "gui/mainwindow.h"
#include "infra/asteriastate.h"
#include "util/timeutil.h"
#include "gui/treeitem.h"
#include "config/configstore.h"
#include "util/v4l2util.h"
#include "infra/imageuc.h"
#include "infra/acquisitionthread.h"
#include "infra/acquisitionvideostats.h"
#include "infra/analysisvideostats.h"
#include "util/testutil.h"

#include <Eigen/Dense>

#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include <QApplication>
#include <QCoreApplication>
#include <QSurfaceFormat>

using namespace std;

static void usage(FILE *fp, char **argv);

static void catchUnixSignals();

int main(int argc, char **argv)
{
    QApplication app (argc, argv);


    // Test the Levenberg-Marquardt fitter
//    TestUtil::testLevenbergMarquardtFitter();
//    TestUtil::testRaDecAzElConversion();

    catchUnixSignals();

    qRegisterMetaType<std::shared_ptr<Imageuc>>("std::shared_ptr<Imageuc>");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<TreeItem>("TreeItem");
    qRegisterMetaType<AcquisitionThread::AcquisitionState>("AcquisitionThread::AcquisitionState");
    qRegisterMetaType<AcquisitionVideoStats>("AcquisitionVideoStats");
    qRegisterMetaType<AnalysisVideoStats>("AnalysisVideoStats");

    // Initialise the state object
    AsteriaState * state = new AsteriaState();

    // Get the time difference between time of day and the frame timestamp. This needs
    // to be recomputed whenever the computer hibernates.
    state->epochTimeDiffUs = TimeUtil::getEpochTimeShift();

    // Parse application parameters from the command line
    static struct option long_options[] =
    {
          {"help",      no_argument,       NULL,             'h'},
          {"cameras",   no_argument,       NULL,             'a'},
          /* These options set a flag. */
          {"headless",  no_argument,       &state->headless,  1},
          {"gui",       no_argument,       &state->headless,  0},
          /* These options don’t set a flag.  We distinguish them by their indices. */
          {"camera",    required_argument, NULL,              'b'},
          {"config",    required_argument, NULL,              'c'},
          {0,           0,                 NULL,               0}
    };

    /* getopt_long stores the option index here. */
    int option_index = 0;

    // Parsed values of the camera and config command line arguments
    char * camera = NULL;
    char * config = NULL;

    int c;
    // The colon after the character indicates that an argument follows
    while ((c = getopt_long (argc, argv, "hab:c:", long_options, &option_index)) != -1) {

        switch (c) {
            case 0: {
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0) {
                    fprintf(stderr, "Mode = %s\n", long_options[option_index].name);
                }
                break;
            }
            case 'h': {
                usage(stderr, argv);
                exit(0);
                break;
            }
            case 'a': {
                vector< pair< string, string > > cameras = V4L2Util::getSupportedV4LCameras(AsteriaState::preferredFormats, AsteriaState::preferredFormatsN);
                fprintf(stderr, "Available cameras:\n");
                for(unsigned int p=0; p<cameras.size(); p++) {
                    fprintf(stderr, "%s (%s)\n", cameras[p].first.c_str(), cameras[p].second.c_str());
                }
                exit(0);
                break;
            }
            case 'b': {
                camera = optarg;
                fprintf(stderr, "Camera = %s\n", camera);
                break;
            }
            case 'c': {
                config = optarg;
                fprintf(stderr, "Config = %s\n", config);
                break;
            }
            case '?': {
                // getopt_long already printed an option
                break;
            }
            default: {
                usage(stderr, argv);
                exit(0);
            }
        }
    }

    // Consistency checks on the arguments
    if(state->headless && !config) {
        fprintf(stderr, "Headless mode: the config file must be specified!\n");
        exit(0);
    }
    if(state->headless && !camera) {
        fprintf(stderr, "Headless mode: the camera must be specified!\n");
        exit(0);
    }
    if(config && !camera) {
        fprintf(stderr, "If config is specified then camera must also be!\n");
        exit(0);
    }

    // Create the GUI elements. We only show the ones we need to.
    CameraSelectionWindow camWin(0, state);
    ConfigCreationWindow configWin(0, state);
    MainWindow mainWin(0, state);

    // Implements advancing from config window to main window
    QObject::connect(&configWin, SIGNAL (ok()), &mainWin, SLOT (initAndShowGui()));
    // Implements advancing from camera window to config window
    QObject::connect(&camWin, SIGNAL (cameraSelected(string)), &configWin, SLOT (show()));
    // Implements returning from config window to camera window
    QObject::connect(&configWin, SIGNAL (cancel()), &camWin, SLOT (show()));

    // If camera is specified then attempt to open a connection
    if(camera) {

        // Attempt to connect to camera; bypass the camera selection GUI
        state->cameraPath = string(camera);
        V4L2Util::openCamera(state->cameraPath, state->fd, state->selectedFormat);

        fprintf(stderr, "Selected camera = %s\n", V4L2Util::getCameraName(*(state->fd)).c_str());
        fprintf(stderr, "Selected pixel format = %s\n", V4L2Util::getFourCC(state->selectedFormat).c_str());

        if(!config) {
            // No config file specified - display config creation window (only reach this point if we're in GUI mode)
            configWin.show();
        }
    }
    else {
        // No camera specified - display camera selection window (only reach this point if we're in GUI mode)
        camWin.show();
    }

    // If config is specified then attempt to parse the file
    if(config) {

        string configFile = string(config);
        // Parse the config file
        ConfigStore store(state);
        store.loadFromFile(configFile);

        // Check the validity of all parameters
        bool allValid = true;
        for(unsigned int famOff = 0; famOff < store.numFamilies; famOff++) {

            fprintf(stderr, "Validating %s parameters...\n", store.families[famOff]->title.c_str());

            for(unsigned int parOff = 0; parOff < store.families[famOff]->numPar; parOff++) {
                // Get the parameter
                ConfigParameterBase * par = store.families[famOff]->parameters[parOff];
                if(par->isValid == INVALID) {
                    allValid = false;
                    fprintf(stderr, "Parameter %s (%s) INVALID: %s\n", par->key.c_str(), par->value.c_str(), par->message.c_str());
                }
                if(par->isValid == WARNING) {
                    fprintf(stderr, "Parameter %s (%s) WARNING: %s\n", par->key.c_str(), par->value.c_str(), par->message.c_str());
                }
            }
        }

        if(!allValid) {
            fprintf(stderr, "Invalid parameters detected, can't start Asteria!\n");
            exit(0);
        }

        // All parameters OK.
        if(state->headless) {
            // Headless mode
            // TODO: introduce a small class to encapsulate the thread and close it down cleanly etc
            AcquisitionThread * acqThread = new AcquisitionThread(0, state);
            QObject::connect(qApp, SIGNAL(aboutToQuit()), acqThread, SLOT(shutdown()));
            acqThread->launch();
        }
        else {
            // Present GUI
            mainWin.initAndShowGui();
        }
    }

    return app.exec();
}

static void usage(FILE *fp, char **argv)
{
        fprintf(fp,
                 "Usage: %s [options]\n\n"
                 "Options:\n"
                 "-h, --help          Print this message\n"
                 "-a, --cameras       Print list of available (supported) cameras\n"
                 "    --headless      Operate in headless (no GUI) mode\n"
                 "    --gui           Operate in GUI mode\n"
                 "-b, --camera PATH   Use the camera located at PATH (e.g. /dev/video0)\n"
                 "-c, --config PATH   Use the asteria.config file located at PATH\n"
                 "",
                 argv[0]);
}

/**
 * Intercept and handle UNIX terminal signals. See https://gist.github.com/azadkuh/a2ac6869661ebd3f8588.
 * @brief catchUnixSignals
 */
static void catchUnixSignals() {

    // SIGQUIT (ctrl \): by default, this causes the process to terminate and dump core.
    // SIGINT (ctrl c): by default, this causes the process to terminate.
    // SIGTSTP (ctrl z): by default, this causes the process to suspend execution.
    // SIGTERM : The SIGTERM signal is sent to a process to request its termination
    // SIGHUP : The SIGHUP signal is sent to a process when its controlling terminal is closed.
    const std::vector<int> quitSignals = {SIGQUIT, SIGINT, SIGTSTP, SIGTERM, SIGHUP, SIGSTOP, SIGSEGV, SIGABRT};

    for (unsigned i=0; i < quitSignals.size(); i++) {
        auto handler = [](int sig) ->void {
            fprintf(stderr, "Received %s signal; quitting...\n", strsignal(sig));
            QCoreApplication::quit();
        };
        signal(quitSignals[i], handler);
    }
}
