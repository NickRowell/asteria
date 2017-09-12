#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <string>
#include <vector>
#include <time.h>
#include <math.h>
#include <regex>
#include <sys/time.h>

class TimeUtil {

public:
    TimeUtil();

    static const char * utcFormatString;
    static const std::regex timeRegex;
    static const std::regex utcRegex;

    static long long getUpTime();
    static long long getEpochTimeShift();

    static void epochToUtc(const long long &epochTimeStamp_us, int &year, int &month, int &day, int &hour, int &min, double &sec);
    static double epochToJd(const long long &epochTimeStamp_us);
    static double epochToGmst(const long long &epochTimeStamp_us);


    static std::string epochToUtcString(const long long &epochTimeStamp_us);
    static std::string extractYearFromUtcString(const std::string &utc);
    static std::string extractMonthFromUtcString(const std::string &utc);
    static std::string extractDayFromUtcString(const std::string &utc);
    static std::string extractTimeFromUtcString(const std::string &utc);

};

#endif // TIMEUTIL_H
