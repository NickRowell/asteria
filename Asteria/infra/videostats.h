#ifndef VIDEOSTATS_H
#define VIDEOSTATS_H

#include <string>

/**
 * @brief The VideoStats class
 * Used to encapsulate various statistics of the current video stream state.
 */
class VideoStats
{

public:
    VideoStats();
    VideoStats(const VideoStats &copyme);
    VideoStats(const double &fps, const unsigned int &droppedFrames, const unsigned int &totalFrames, const std::string &utc);

    double fps;

    unsigned int droppedFrames;

    unsigned int totalFrames;

    std::string utc;

};

#endif // VIDEOSTATS_H
