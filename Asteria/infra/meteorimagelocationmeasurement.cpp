#include "meteorimagelocationmeasurement.h"

MeteorImageLocationMeasurement::MeteorImageLocationMeasurement() :
    changedPixelsPositive(0), changedPixelsNegative(0) {

    // No point using initialiser list for PODs
    epochTimeUs = 0ll;
    coarse_localisation_success = false;
    bb_xmin = 0;
    bb_xmax = 0;
    bb_ymin = 0;
    bb_ymax = 0;
    x_flux_centroid = 0.0;
    y_flux_centroid = 0.0;

}

MeteorImageLocationMeasurement::MeteorImageLocationMeasurement(const MeteorImageLocationMeasurement & copyme) :
changedPixelsPositive(copyme.changedPixelsPositive), changedPixelsNegative(copyme.changedPixelsNegative) {

    // No point using initialiser list for PODs
    epochTimeUs = copyme.epochTimeUs;
    coarse_localisation_success = copyme.coarse_localisation_success;
    bb_xmin = copyme.bb_xmin;
    bb_xmax = copyme.bb_xmax;
    bb_ymin = copyme.bb_ymin;
    bb_ymax = copyme.bb_ymax;
    x_flux_centroid = copyme.x_flux_centroid;
    y_flux_centroid = copyme.y_flux_centroid;

}

MeteorImageLocationMeasurement& MeteorImageLocationMeasurement::operator =(const MeteorImageLocationMeasurement & copyme) {

    changedPixelsPositive = copyme.changedPixelsPositive;
    changedPixelsNegative = copyme.changedPixelsNegative;
    epochTimeUs = copyme.epochTimeUs;
    coarse_localisation_success = copyme.coarse_localisation_success;
    bb_xmin = copyme.bb_xmin;
    bb_xmax = copyme.bb_xmax;
    bb_ymin = copyme.bb_ymin;
    bb_ymax = copyme.bb_ymax;
    x_flux_centroid = copyme.x_flux_centroid;
    y_flux_centroid = copyme.y_flux_centroid;

    return *this;
}

bool MeteorImageLocationMeasurement::operator < (const MeteorImageLocationMeasurement& loc) const {
    return (epochTimeUs < loc.epochTimeUs);
}
