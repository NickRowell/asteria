#include "videostats.h"

VideoStats::VideoStats() {

}

VideoStats::VideoStats(const VideoStats &copyme) : fps(copyme.fps), droppedFrames(copyme.droppedFrames), totalFrames(copyme.totalFrames) {

}

VideoStats::VideoStats(const double &fps, const unsigned int &droppedFrames, const unsigned int &totalFrames) :
    fps(fps), droppedFrames(droppedFrames), totalFrames(totalFrames) {

}
