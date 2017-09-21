#ifndef METEORCAPTURESTATE_H
#define METEORCAPTURESTATE_H

#include "infra/referencestar.h"
#include <linux/videodev2.h>
#include <string>
#include <vector>
#include <cstring>              // memset

using namespace std;


class AsteriaState
{

public:

    // Global variables. See .cpp for details of each.
    static const unsigned int preferredFormats[];
    static const unsigned int preferredFormatsN;

    /**
     * @brief MeteorCaptureState
     */
    AsteriaState();

    ~AsteriaState();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                    Application parameters                    //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Application parameters (read from command line, affect behaviour of application)
    // e.g. run headless
    //      config directory location
    /**
     * \brief Boolean flag to indicate if we're running without a GUI.
     */
    int headless = 0;

    // Chosen pixel format
    unsigned int selectedFormat;

    /**
     * @brief configDirPath
     * Directory where calibration info is to be stored
     */
    string configDirPath;

    /**
     * \brief Path to the camera device file
     */
    string cameraPath;

    /**
     * \brief Open file descriptor on the camera device
     */
    int * fd;

    // Cannot be loaded from config file: must be created programmatically,
    // either by user selection or automated selection of default camera.

    /**
     * @brief epochTimeDiffUs
     * Time difference between system clock time (since startup/hibernation) and the
     * current epoch time. Used to convert image time codes to UTC.
     */
    long long epochTimeDiffUs;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                      Camera parameters                       //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * \brief The image width
     */
    unsigned int width;

    /**
     * \brief The image height
     */
    unsigned int height;

    /**
     * \brief Camera azimuthal angle [decimal degrees]
     */
    double azimuth;

    /**
     * \brief Camera elevation angle [decimal degrees]
     */
    double elevation;

    /**
     * \brief Camera roll angle [decimal degrees]
     */
    double roll;

    // Cannot be loaded from config file: must be created programmatically,
    // either by user selection or automated selection of default camera.

    /**
     * @brief frameperiodUs
     * Nominal interval between frames in microseconds. The actual period as determined by the
     * capture times of individual frames may vary by up to 10% or so.
     */
    unsigned int nominalFramePeriodUs;

    /**
     * @brief nominalExposureTimeUs
     * Nominal exposure time for frames in microseconds.
     */
    unsigned int nominalExposureTimeUs;


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                      System parameters                       //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief videoDirPath
     */
    string videoDirPath;

    /**
     * @brief calibrationDirPath
     */
    string calibrationDirPath;

    /**
     * @brief refStarCataloguePath
     */
    string refStarCataloguePath;

    /**
     * @brief refStarCatalogue
     */
    vector<ReferenceStar> refStarCatalogue;

    /**
     * @brief jplEphemerisPath
     */
    string jplEphemerisPath;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                      Station parameters                      //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * \brief Station longitude, positive east [decimal degrees]
     */
    double longitude;

    /**
     * \brief Station latitude [decimal degrees]
     */
    double latitude;

    /**
     * \brief Station altitude [metres]
     */
    double altitude;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                     Detection parameters                     //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Number of frames to buffer for head of each detection, i.e.
     * before the event started.
     */
    unsigned int detection_head;

    /**
     * @brief Number of frames to buffer for tail of each detection, i.e.
     * after the event has ceased.
     */
    unsigned int detection_tail;

    /**
     * @brief Maximum clip length, excluding head [minutes]
     */
    double clip_max_length;

    /**
     * @brief Difference between the digital levels of a pixel between frames that indicate
     * a significant change, i.e. one that counts towards an event trigger.
     */
    unsigned int pixel_difference_threshold;

    /**
     * @brief Number of significantly changed pixels required to trigger an event detection.
     */
    unsigned int n_changed_pixels_for_trigger;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                     Analysis parameters                      //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * Limit on the deviation from a straight line for a detection to be classified as a meteor.
     * @brief linearity_threshold
     */
    double linearity_threshold;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                   Calibration parameters                     //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Period between calibration routine executions [minutes]
     */
    double calibration_interval;

    /**
     * @brief Number of frames that are stacked to produce the calibration images [frames]
     */
    unsigned int calibration_stack;

    /**
     * @brief Half-width of the median filter kernel used to estimate the background image [pixels]. The
     * full size kernel is (2N+1)x(2N+1).
     */
    unsigned int bkg_median_filter_half_width;

    /**
     * @brief Threshold for detection of significant sources, in terms of the number of standard deviations
     * that the integrated flux lies above the background level [dimensionless].
     */
    double source_detection_threshold_sigmas;


};

#endif // METEORCAPTURESTATE_H
