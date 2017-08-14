#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <vector>

class MathUtil
{
public:
    MathUtil();

   static void getMedianMad(std::vector<unsigned int> values, int &median, int &mad);

};

#endif // MATHUTIL_H
