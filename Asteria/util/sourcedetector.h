#ifndef SOURCEDETECTOR_H
#define SOURCEDETECTOR_H

#include "infra/source.h"
#include "infra/sample.h"

#include <vector>
#include <set>

class SourceDetector
{
public:
    SourceDetector();

    static std::vector<Source> getSources(std::vector<unsigned char> &median, std::vector<unsigned char> &background, std::vector<unsigned char> &noise,
                                          unsigned int &width, unsigned int &height, double &source_detection_threshold_sigmas);
    static std::vector<unsigned int> getNeighbourUniqueLabels(Sample *&sample, const std::vector<Sample *> &samples, unsigned int &width, unsigned int &height);
};

#endif // SOURCEDETECTOR_H
