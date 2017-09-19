#include "mathutil.h"

#include <algorithm>

MathUtil::MathUtil()
{

}


void MathUtil::getMedianMad(std::vector<unsigned int> values, int &median, int &mad) {

    size_t size = values.size();

    std::sort(values.begin(), values.end());

    if (size  % 2 == 0) {
        median = (values[size / 2 - 1] + values[size / 2]) / 2;
    }
    else {
        median = values[size / 2];
    }

    // Vector of absolute deviations from the median
    std::vector<unsigned int> absDevs;
    for(unsigned int p = 0; p < values.size(); ++p) {
        absDevs.push_back(abs(values[p] - median));
    }

    std::sort(absDevs.begin(), absDevs.end());

    if (size  % 2 == 0) {
        mad = (absDevs[size / 2 - 1] + absDevs[size / 2]) / 2;
    }
    else {
        mad = absDevs[size / 2];
    }

}

double MathUtil::toDegrees(const double &rad) {
    return rad * (180.0 / M_PI);
}

double MathUtil::toRadians(const double &deg) {
    return deg * (M_PI / 180.0);
}
