#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <string>
#include <time.h>
#include <math.h>
#include <sys/time.h>

class TimeUtil
{
public:
    TimeUtil();

    static long long getUpTime();

    static long long getEpochTimeShift();

    static std::string convertToUtcString(long long epochTimeStamp_us);


};

#endif // TIMEUTIL_H
