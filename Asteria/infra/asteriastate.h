#ifndef METEORCAPTURESTATE_H
#define METEORCAPTURESTATE_H

#include "infra/referencestar.h"
#include <linux/videodev2.h>
#include <string>
#include <vector>
#include <cstring>

class CalibrationInventory;

using namespace std;

/**
 * @brief Encapsulates all the parameters of the current instance of the Asteria application and maintains
 * certain global fields that are shared among different processes, such as the camera calibration.
 */
class AsteriaState
{

public:

    // Global variables. See .cpp for details of each.
    static const unsigned int preferredFormats[];
    static const unsigned int preferredFormatsN;

    /**
     * @brief Constructor for the AsteriaState
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
     * @brief Boolean flag to indicate if we're running without a GUI.
     */
    int headless = 0;

    /**
     * @brief Chosen pixel format
     */
    unsigned int selectedFormat;

    /**
     * @brief Directory where calibration info is to be stored
     */
    string configDirPath;

    /**
     * @brief Path to the camera device file
     */
    string cameraPath;

    /**
     * @brief Open file descriptor on the camera device
     */
    int * fd;

    /**
     * @brief The camera calibration data currently in use for processing new events. By default this is the most
     * recent found in the calibration directory, or NULL if none exists.
     */
    CalibrationInventory * cal;

    // Cannot be loaded from config file: must be created programmatically,
    // either by user selection or automated selection of default camera.

    /**
     * @brief Time difference between system clock time (since startup/hibernation) and the
     * current epoch time. Used to convert image time codes to UTC.
     */
    long long epochTimeDiffUs;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                      Camera parameters                       //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief The image width
     */
    unsigned int width;

    /**
     * @brief The image height
     */
    unsigned int height;

    /**
     * @brief Lens focal length [millimetres]
     */
    double focal_length;

    /**
     * @brief Pixel horiontal width [micrometres]
     */
    double pixel_width;

    /**
     * @brief Pixel vertical height [micrometres]
     */
    double pixel_height;

    /**
     * @brief Camera azimuthal angle, measured east of north [decimal degrees]
     */
    double azimuth;

    /**
     * @brief Camera elevation angle, measured from horizon [decimal degrees]
     */
    double elevation;

    /**
     * @brief Camera roll angle, clockwise around the boresight [decimal degrees]
     */
    double roll;

    // Cannot be loaded from config file: must be created programmatically,
    // either by user selection or automated selection of default camera.

    /**
     * @brief Nominal interval between frames in microseconds. The actual period as determined by the
     * capture times of individual frames may vary by up to 10% or so.
     */
    unsigned int nominalFramePeriodUs;

    /**
     * @brief Nominal exposure time for frames in microseconds.
     */
    unsigned int nominalExposureTimeUs;


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                      System parameters                       //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Directory to store raw and processed data from detected events.
     */
    string videoDirPath;

    /**
     * @brief Directory to store generated system calibration data.
     */
    string calibrationDirPath;

    /**
     * @brief Path to the reference star catalogue.
     */
    string refStarCataloguePath;

    /**
     * @brief The loaded contents of the reference star catalogue.
     */
    vector<ReferenceStar> refStarCatalogue;

    /**
     * @brief Path to the JPL Earth ephemeris.
     */
    string jplEphemerisPath;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                      Station parameters                      //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Station longitude, positive east [decimal degrees]
     */
    double longitude;

    /**
     * @brief Station latitude [decimal degrees]
     */
    double latitude;

    /**
     * @brief Station altitude [metres]
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
     * @brief Limit on the deviation from a straight line for a detection to be classified as a meteor.
     */
    double linearity_threshold;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                              //
    //                   Calibration parameters                     //
    //                                                              //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief The name of the type of camera model to be used.
     */
    string camera_model_type;

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

    /**
     * @brief Faint visual magnitude limit for reference stars used in the calibration [mags]
     */
    double ref_star_faint_mag_limit;

};

#endif // ASTERIASTATE_H
