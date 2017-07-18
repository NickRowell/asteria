#include "analysisvideostats.h"

AnalysisVideoStats::AnalysisVideoStats()
{

}

AnalysisVideoStats::AnalysisVideoStats(const AnalysisVideoStats &copyme) :
    clipLengthSecs(copyme.clipLengthSecs), clipLengthFrames(copyme.clipLengthFrames), framePositionSecs(copyme.framePositionSecs),
    framePositionFrames(copyme.framePositionFrames), isTopField(copyme.isTopField), isBottomField(copyme.isBottomField), utc(copyme.utc){

}

AnalysisVideoStats::AnalysisVideoStats(const double &clipLengthSecs, const unsigned int &clipLengthFrames, const double &framePositionSecs,
                   const unsigned int &framePositionFrames, const bool &isTopField, const bool &isBottomField, const std::string &utc) :
    clipLengthSecs(clipLengthSecs), clipLengthFrames(clipLengthFrames), framePositionSecs(framePositionSecs), framePositionFrames(framePositionFrames),
    isTopField(isTopField), isBottomField(isBottomField), utc(utc) {

}
