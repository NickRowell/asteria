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

    static std::vector<Source> getSources(std::vector<double> &signal, std::vector<double> &background, std::vector<double> &noise,
                                          unsigned int &width, unsigned int &height, double &source_detection_threshold_sigmas);

private:
    static std::vector<unsigned int> getNeighbourUniqueLabels(Sample<double> *&sample, const std::vector<Sample<double> *> &samples, unsigned int &width, unsigned int &height);
};

#endif // SOURCEDETECTOR_H
