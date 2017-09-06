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

    static std::vector<Source> getSources(std::vector<unsigned char> pixels, unsigned int width, unsigned int height);
    static std::vector<unsigned int> getNeighbourUniqueLabels(Sample *&sample, const std::vector<Sample *> &samples, unsigned int &width, unsigned int &height);
};

#endif // SOURCEDETECTOR_H
