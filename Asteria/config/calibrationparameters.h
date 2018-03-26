#ifndef CALIBRATIONPARAMETERS_H
#define CALIBRATIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parametermultiplechoice.h"
#include "config/singleparameter.h"
#include "infra/asteriastate.h"
#include "optics/cameramodelbase.h"

#include <QDebug>

class CalibrationParameters : public ConfigParameterFamily {

public:

    CalibrationParameters(AsteriaState * state) : ConfigParameterFamily("Calibration", 5) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
//        validators[0] = NULL;
        validators[0] = new ValidateWithinLimits<double>(0.0, 10000.0);
        validators[1] = new ValidateWithinLimits<unsigned int>(0u, 100u);
        validators[2] = new ValidateWithinLimits<unsigned int>(0u, 30u);
        validators[3] = new ValidateWithinLimits<double>(0.0, 50.0);
        validators[4] = new ValidateWithinLimits<double>(-1.0, 20.0);

        // Create parameters

        // Vector of string options for camera model types
//        std::vector<string> cameraModelTypeOptions;
//        for(std::vector<CameraModelBase::CameraModelType>::const_iterator it = CameraModelBase::cameraModelTypes.begin();
//            it != CameraModelBase::cameraModelTypes.end(); ++it) {
//            CameraModelBase * cam = CameraModelBase::getCameraModelFromEnum(*it);
//            cameraModelTypeOptions.push_back(cam->getModelName());
//        }

//        parameters[0] = new ParameterMultipleChoice("camera_model_type", "Camera Model Type", cameraModelTypeOptions, &(state->camera_model_type));

        parameters[0] = new SingleParameter<double>("calibration_interval", "Calibration Interval", "minutes", validators[0], &(state->calibration_interval));
        parameters[1] = new SingleParameter<unsigned int>("calibration_stack", "Number of frames used for calibration", "frames", validators[1], &(state->calibration_stack));
        parameters[2] = new SingleParameter<unsigned int>("bkg_median_filter_half_width", "Half-width of median filter kernel for background estimation", "pixels", validators[2], &(state->bkg_median_filter_half_width));
        parameters[3] = new SingleParameter<double>("source_detection_threshold_sigmas", "Source detection threshold, in sigmas above the background level", "-", validators[3], &(state->source_detection_threshold_sigmas));
        parameters[4] = new SingleParameter<double>("ref_star_faint_mag_limit", "Reference star faint magnitude limit", "mag", validators[4], &(state->ref_star_faint_mag_limit));
    }
};

#endif
