#ifndef ANALYSISVIDEOSTATS_H
#define ANALYSISVIDEOSTATS_H

#include <string>

class AnalysisVideoStats
{

public:
    AnalysisVideoStats();
    AnalysisVideoStats(const AnalysisVideoStats &copyme);
    AnalysisVideoStats(const double &clipLengthSecs, const unsigned int &clipLengthFrames, const double &framePositionSecs,
                       const unsigned int &framePositionFrames, const bool &isTopField, const bool &isBottomField, const std::string &utc);

    /**
     * @brief clipLengthSecs
     * The length of the current clip [sec]
     */
    double clipLengthSecs;

    /**
     * @brief clipLengthFrames
     * The length of the current clip [frames]
     */
    unsigned int clipLengthFrames;

    /**
     * @brief framePositionSecs
     * The position of the current frame within the clip [sec]
     */
    double framePositionSecs;

    /**
     * @brief framePositionFrames
     * The position of the current frame within the clip [frames]
     */
    unsigned int framePositionFrames;

    /**
     * @brief isTopField
     * Flag to indicate if the current image is displaying the top field. This is only
     * relevant for interlaced scan images where we can display the top and bottom fields
     * separately; for progressive scan images this is always true.
     */
    bool isTopField;

    /**
     * @brief isBottomField
     * Flag to indicate if the current image is displaying the bottom field. This is only
     * relevant for interlaced scan images where we can display the top and bottom fields
     * separately; for progressive scan images this is always true.
     */
    bool isBottomField;

    /**
     * @brief utc
     * UTC of the current image
     */
    std::string utc;

};

#endif // ANALYSISVIDEOSTATS_H
