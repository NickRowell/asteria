#include "timeutil.h"

#include <sstream>              // ostringstream

TimeUtil::TimeUtil()
{

}

// Microseconds after 1970-01-01T00:00:00Z
std::string TimeUtil::convertToUtcString(long long epochTimeStamp_us) {

    // Split into whole seconds and remainder microseconds
    long long epochTimeStamp_s = epochTimeStamp_us / 1000000LL;
    long long epochTimeStamp_us_remainder = epochTimeStamp_us % 1000000LL;

    // Convert the seconds part to time_t
    time_t tt = static_cast<time_t>(epochTimeStamp_s);

    // Use standard library function(s) to convert this to human readable date/time
    struct tm * ptm = gmtime ( &tt );

    // seconds after the minute	[0-61]*
    // * tm_sec is generally 0-59. The extra range is to accommodate for leap seconds in certain systems.
    int tm_sec = ptm->tm_sec;
    // minutes after the hour	0-59
    int tm_min = ptm->tm_min;
    // hours since midnight	[0-23]
    int tm_hour = ptm->tm_hour;
    // day of the month	[1-31]
    int tm_mday = ptm->tm_mday;
    // months since January	[0-11]
    int tm_mon = ptm->tm_mon;
    // years since 1900; convert to years since AD 0
    int tm_year = ptm->tm_year + 1900;

    // Construct date string
    std::ostringstream strs;
    strs << tm_year << "-" << tm_mon << "-" << tm_mday << "--" << tm_hour << ":" << tm_min << ":" << tm_sec << "." << epochTimeStamp_us_remainder;

    return strs.str();
}
