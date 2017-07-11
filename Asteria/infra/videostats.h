#ifndef VIDEOSTATS_H
#define VIDEOSTATS_H

/**
 * @brief The VideoStats class
 * Used to encapsulate various statistics of the current video stream state.
 */
class VideoStats
{

public:
    VideoStats();
    VideoStats(const VideoStats &copyme);
    VideoStats(const double &fps, const unsigned int &droppedFrames, const unsigned int &totalFrames);

    double fps;

    unsigned int droppedFrames;

    unsigned int totalFrames;

};

#endif // VIDEOSTATS_H
