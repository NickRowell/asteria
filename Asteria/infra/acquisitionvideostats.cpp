#include "acquisitionvideostats.h"

AcquisitionVideoStats::AcquisitionVideoStats() {

}

AcquisitionVideoStats::AcquisitionVideoStats(const AcquisitionVideoStats &copyme) :
    fps(copyme.fps), droppedFrames(copyme.droppedFrames), totalFrames(copyme.totalFrames), utc(copyme.utc) {

}

AcquisitionVideoStats::AcquisitionVideoStats(const double &fps, const unsigned int &droppedFrames, const unsigned int &totalFrames, const std::string &utc) :
    fps(fps), droppedFrames(droppedFrames), totalFrames(totalFrames), utc(utc) {

}
