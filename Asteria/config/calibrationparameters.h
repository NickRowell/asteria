#ifndef CALIBRATIONPARAMETERS_H
#define CALIBRATIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parametermultiplechoice.h"
#include "config/parametersingle.h"
#include "infra/asteriastate.h"
#include "optics/cameramodelbase.h"

#include <QDebug>

class CalibrationParameters : public ConfigParameterFamily {

public:

    CalibrationParameters(AsteriaState * state) : ConfigParameterFamily("Calibration", 6) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
        validators[0] = NULL;
        validators[1] = new ValidateWithinLimits<double>(0.0, 10000.0);
        validators[2] = new ValidateWithinLimits<unsigned int>(0u, 100u);
        validators[3] = new ValidateWithinLimits<unsigned int>(0u, 30u);
        validators[4] = new ValidateWithinLimits<double>(0.0, 50.0);
        validators[5] = new ValidateWithinLimits<double>(-1.0, 20.0);

        // Create parameters

        // Vector of string options for camera model types
        std::vector<string> cameraModelTypeOptions;
        for(std::vector<CameraModelBase::CameraModelType>::const_iterator it = CameraModelBase::cameraModelTypes.begin();
            it != CameraModelBase::cameraModelTypes.end(); ++it) {
            CameraModelBase * cam = CameraModelBase::getCameraModelFromEnum(*it);
            cameraModelTypeOptions.push_back(cam->getModelName());
        }

        parameters[0] = new ParameterMultipleChoice<string>("camera_model_type", "Camera Model Type", cameraModelTypeOptions, &(state->camera_model_type));
        parameters[1] = new ParameterSingle<double>("calibration_interval", "Calibration Interval", "minutes", validators[1], &(state->calibration_interval));
        parameters[2] = new ParameterSingle<unsigned int>("calibration_stack", "Number of frames used for calibration", "frames", validators[2], &(state->calibration_stack));
        parameters[3] = new ParameterSingle<unsigned int>("bkg_median_filter_half_width", "Half-width of median filter kernel for background estimation", "pixels", validators[3], &(state->bkg_median_filter_half_width));
        parameters[4] = new ParameterSingle<double>("source_detection_threshold_sigmas", "Source detection threshold, in sigmas above the background level", "-", validators[4], &(state->source_detection_threshold_sigmas));
        parameters[5] = new ParameterSingle<double>("ref_star_faint_mag_limit", "Reference star faint magnitude limit", "mag", validators[5], &(state->ref_star_faint_mag_limit));
    }
};

#endif
