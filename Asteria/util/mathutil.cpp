#include "mathutil.h"

#include <algorithm>

MathUtil::MathUtil()
{

}

unsigned char MathUtil::getMedian(std::vector<unsigned char> &values) {

    std::sort(values.begin(), values.end());

    if(values.size() % 2 == 0) {
        // Even number of elements - take average of central two
        unsigned int a = (int)values[values.size()/2];
        unsigned int b = (int)values[values.size()/2 - 1];
        unsigned int c = (a + b)/2;
        return (unsigned char)(c & 0xFF);
    }
    else {
        // Odd number of elements - pick central one
        return values[values.size()/2];
    }
}

double MathUtil::getMedian(std::vector<double> &values) {

    std::sort(values.begin(), values.end());

    if(values.size() % 2 == 0) {
        // Even number of elements - take average of central two
        double a = values[values.size()/2];
        double b = values[values.size()/2 - 1];
        return (a + b)/2.0;
    }
    else {
        // Odd number of elements - pick central one
        return values[values.size()/2];
    }
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

void MathUtil::getTrimmedMeanStd(std::vector<double> values, double &mean, double &std, const double percentile) {

    // Sort the vector
    std::sort(values.begin(), values.end());

    // Get the number of outliers to exclude from each end of range
    unsigned int outliers = static_cast<unsigned int>(percentile * values.size());
    double inliers = static_cast<double>(values.size() - 2*outliers);

    mean = 0.0;
    double mean_of_square = 0.0;

    for(unsigned int i=outliers; i < values.size() - outliers; i++) {
        mean += values[i];
        mean_of_square += values[i]*values[i];
    }
    mean /= inliers;
    mean_of_square /= inliers;
    std = std::sqrt(mean_of_square - mean*mean);
}


double MathUtil::toDegrees(const double &rad) {
    return rad * (180.0 / M_PI);
}

double MathUtil::toRadians(const double &deg) {
    return deg * (M_PI / 180.0);
}
