#ifndef CALIBRATIONPARAMETERS_H
#define CALIBRATIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameterdouble.h"
#include "config/parameteruint.h"
#include "infra/asteriastate.h"

#include <QDebug>

class CalibrationParameters : public ConfigParameterFamily {

public:

    CalibrationParameters(AsteriaState * state) : ConfigParameterFamily("Calibration", 4) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
        validators[0] = new ValidateWithinLimits<double>(0.0, 10000.0);
        validators[1] = new ValidateWithinLimits<unsigned int>(0u, 100u);
        validators[2] = new ValidateWithinLimits<unsigned int>(0u, 30u);
        validators[3] = new ValidateWithinLimits<double>(0.0, 50.0);

        // Create parameters
        parameters[0] = new ParameterDouble("calibration_interval", "Calibration Interval", "minutes", validators[0], &(state->calibration_interval));
        parameters[1] = new ParameterUint("calibration_stack", "Number of frames used for calibration", "frames", validators[1], &(state->calibration_stack));
        parameters[2] = new ParameterUint("bkg_median_filter_half_width", "Half-width of median filter kernel for background estimation", "pixels", validators[2], &(state->bkg_median_filter_half_width));
        parameters[3] = new ParameterDouble("source_detection_threshold_sigmas", "Source detection threshold, in sigmas above the background level", "-", validators[3], &(state->source_detection_threshold_sigmas));
    }
};

#endif
