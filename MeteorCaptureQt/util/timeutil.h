#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <string>
#include <vector>
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

    static void convertYearMonthDayHourMinSecUsecString(long long epochTimeStamp_us, std::vector<std::string>& str);
};

#endif // TIMEUTIL_H
