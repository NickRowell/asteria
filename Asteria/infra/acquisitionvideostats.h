#ifndef ACQUISITIONVIDEOSTATS_H
#define ACQUISITIONVIDEOSTATS_H

#include <string>

/**
 * @brief The AcquisitionVideoStats class
 * Used to encapsulate various statistics of the current acquisition video stream state.
 */
class AcquisitionVideoStats
{

public:
    AcquisitionVideoStats();
    AcquisitionVideoStats(const AcquisitionVideoStats &copyme);
    AcquisitionVideoStats(const double &fps, const unsigned int &droppedFrames, const unsigned int &totalFrames, const std::string &utc);

    /**
     * @brief fps
     * The current FPS
     */
    double fps;

    /**
     * @brief droppedFrames
     * The number of dropped frames so far
     */
    unsigned int droppedFrames;

    /**
     * @brief totalFrames
     * Number of frames captured so far
     */
    unsigned int totalFrames;

    /**
     * @brief utc
     * UTC of the current image
     */
    std::string utc;

};

#endif // ACQUISITIONVIDEOSTATS_H
