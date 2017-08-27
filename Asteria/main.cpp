#include "gui/cameraselectionwindow.h"
#include "gui/configcreationwindow.h"
#include "gui/mainwindow.h"
#include "infra/asteriastate.h"
#include "util/timeutil.h"
#include "gui/treeitem.h"
#include "config/configstore.h"
#include "util/v4l2util.h"
#include "infra/image.h"
#include "infra/acquisitionthread.h"
#include "infra/acquisitionvideostats.h"
#include "infra/analysisvideostats.h"

#include "math/cosinefitter.h"

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


    // TEMP: test the Levenberg-Marquardt fitter
    double amp = 2.0;
    double freq = 1.312;
    double phase = 0.25;
    std::vector<double> xs;
    std::vector<double> ys;
    for(double x=0; x<2*M_PI; x+=M_PI/180) {
        double y = amp * std::cos(x * freq + phase);
        xs.push_back(x);
        ys.push_back(y);
    }

    CosineFitter cosFit(xs, ys);
    double initialGuessParams[3];
    initialGuessParams[0] = 4.5;
    initialGuessParams[1] = 3.720;
    initialGuessParams[2] = 0.45;
    cosFit.setParameters(initialGuessParams);
    cosFit.fit(125, true);
    double solution[3];
    cosFit.getParameters(solution);
    fprintf(stderr, "Amplitude = %f\n", solution[0]);
    fprintf(stderr, "Frequency = %f\n", solution[1]);
    fprintf(stderr, "Phase     = %f\n", solution[2]);
    // Print the data and model
    double model[xs.size()];
    cosFit.getModel(solution, model);
    double initialModel[xs.size()];
    cosFit.getModel(initialGuessParams, initialModel);
    for(unsigned int i=0; i<xs.size(); i++) {
        fprintf(stderr, "%f\t%f\t%f\t%f\n", xs[i], ys[i], model[i], initialModel[i]);
    }





    catchUnixSignals();

    qRegisterMetaType<std::shared_ptr<Image>>("std::shared_ptr<Image>");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<TreeItem>("TreeItem");
    qRegisterMetaType<AcquisitionThread::AcquisitionState>("AcquisitionThread::AcquisitionState");
    qRegisterMetaType<AcquisitionVideoStats>("AcquisitionVideoStats");
    qRegisterMetaType<AnalysisVideoStats>("AnalysisVideoStats");

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
