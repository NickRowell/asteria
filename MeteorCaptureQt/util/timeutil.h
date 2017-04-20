#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <string>

class TimeUtil
{
public:
    TimeUtil();

    static std::string convertToUtcString(long long epochTimeStamp_us);


};

#endif // TIMEUTIL_H
