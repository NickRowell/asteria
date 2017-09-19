#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <vector>

class MathUtil
{
public:
    MathUtil();

   static void getMedianMad(std::vector<unsigned int> values, int &median, int &mad);

   static double toDegrees(const double &rad);
   static double toRadians(const double &deg);
};

#endif // MATHUTIL_H
