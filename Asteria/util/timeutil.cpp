#include "timeutil.h"
#include "util/mathutil.h"

#include <sstream>

TimeUtil::TimeUtil() {

}

// Format spec for string representation of UTC
const char * TimeUtil::utcFormatString = "%04d-%02d-%02dT%02d:%02d:%06.3fZ";

// Regex suitable for identifying strings containing e.g. 19:41:09
const std::regex TimeUtil::timeRegex("[0-9]{2}:[0-9]{2}:[0-9]{2}");

// Regex suitable for identifying strings containing e.g. 2017-06-14T19:41:09.282Z
const std::regex TimeUtil::utcRegex("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}Z");

long long TimeUtil::getUpTime() {

    // Records time since bootup, to nanosecond accuracy
    struct timespec  vsTime;

    clock_gettime(CLOCK_MONOTONIC, &vsTime);

    long long uptime_us = vsTime.tv_sec* 1000000LL + (long long)  round( vsTime.tv_nsec/ 1000.0);
    return uptime_us;
}

long long TimeUtil::getEpochTimeShift() {

    // Records time of day, to microsecond accuracy
    struct timeval epochtime;

    // Records time since bootup, to nanosecond accuracy
    struct timespec  vsTime;

    gettimeofday(&epochtime, NULL);
    clock_gettime(CLOCK_MONOTONIC, &vsTime);

    long long uptime_us = vsTime.tv_sec* 1000000LL + (long long)  round( vsTime.tv_nsec/ 1000.0);
    long long epoch_us =  epochtime.tv_sec * 1000000LL  + (long long) round( epochtime.tv_usec);
    return epoch_us - uptime_us;
}

void TimeUtil::epochToUtc(const long long &epochTimeStamp_us, int &year, int &month, int &day, int &hour, int &min, double &sec) {

    // Split into whole seconds and remainder microseconds
    long long epochTimeStamp_s = epochTimeStamp_us / 1000000LL;
    long long epochTimeStamp_us_remainder = epochTimeStamp_us % 1000000LL;

    // Convert the seconds part to time_t
    time_t tt = static_cast<time_t>(epochTimeStamp_s);

    // Use standard library function(s) to convert this to human readable date/time
    struct tm * ptm = gmtime ( &tt );

    year = ptm->tm_year;     // years since 1900
    month = ptm->tm_mon;     // months since January [0-11]
    day = ptm->tm_mday;      // day of the month [1-31]
    hour = ptm->tm_hour;     // hours since midnight [0-23]
    min = ptm->tm_min;       // minutes after the hour [0-59]
    int isec = ptm->tm_sec;  // seconds after the minute [0-61]
    // Add the remainder seconds onto the whole seconds
    sec = (double)isec + (double)(epochTimeStamp_us_remainder) / 1000000.0;

    // Convert to years since AD 0
    year += 1900;
    // Convert months to 1-12
    month += 1;

    return;
}

void TimeUtil::utcToEpoch(long long &epochTime_us, const int &year, const int &month, const int &day, const int &hour, const int &min, const double &sec) {

    // Create tm struct and initialize to all 0's
    struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = (int)sec;

    time_t epochTime_s = mktime(&t);

    // This is in seconds; convert to microseconds
    epochTime_us = epochTime_s * 1000000LL;

    // Get the fractional part of the seconds and convert to microseconds
    long long usec = (long long)((sec - (int)sec) * 1000000.0);

    epochTime_us += usec;

    return;
}

double TimeUtil::epochToJd(const long long &epochTimeStamp_us) {
    // The Unix epoch (zero-point) is January 1, 1970 GMT. That corresponds to the Julian day of 2440587.5
    // We just need to add the number of consecutive days since then, ignoring leap seconds.
    // The constant 86400000000.0 is the number of microseconds in a day.
    return 2440587.5 + epochTimeStamp_us/86400000000.0;
}

double TimeUtil::epochToGmst(const long long &epochTimeStamp_us) {

    // Julian centuries since 2000 Jan. 1 12h UT1
    double t = (epochToJd(epochTimeStamp_us) - 2451545.0) / 36525.0;

    // Compute the GMST in seconds
    double gmst = 67310.54841 + (876600.0*60.0*60.0 + 8640184.812866) * t + (0.093104 * t * t) - (0.0000062 * t * t * t);

    // Shift to [0:86400] range
    while(gmst < 0.0) {
        gmst += 86400.0;
    }
    while(gmst > 86400.0) {
        gmst -= 86400.0;
    }

    // Convert to decimal days:
    double gmst_days = gmst / 86400.0;

    // Convert to decimal hours
    double gmst_hours = gmst_days * 24.0;

    return gmst_hours;
}

double TimeUtil::gmstToLst(const double &gmst, const double &longitude) {
    // Convert longitude to hour angle
    double ha = MathUtil::toDegrees(longitude) / 15.0;
    // Add offset to GMST
    double lst = gmst + ha;
    return lst;
}

void TimeUtil::decimalHoursToHMS(const double &dhour, int &hour, int &min, double &sec) {
    // Whole hours
    hour = (int)std::floor(dhour);
    // Decimal minutes from remainder
    double dmin = (dhour - std::floor(dhour)) * 60.0;
    // Whole mins
    min = (int)std::floor(dmin);
    // Decimal seconds from remainder
    sec = (dmin - std::floor(dmin)) * 60.0;
}

std::string TimeUtil::epochToUtcString(const long long &epochTimeStamp_us) {

    int year, month, day, hour, min;
    double sec;
    epochToUtc(epochTimeStamp_us, year, month, day, hour, min, sec);

    // Construct date string
    char utcStr[100];
    sprintf (utcStr, utcFormatString, year, month, day, hour, min, sec);

    std::string utcStrr(utcStr);
    return utcStrr;
}

long long TimeUtil::utcStringToEpoch(const std::string &utcString) {

    // Extract the year, month, day, hour, minute and seconds from the string and parse to numbers
    int year = std::stoi(extractYearFromUtcString(utcString));
    int month = std::stoi(extractMonthFromUtcString(utcString));
    int day = std::stoi(extractDayFromUtcString(utcString));
    int hour = std::stoi(extractHourFromUtcString(utcString));
    int min = std::stoi(extractMinFromUtcString(utcString));
    double secs = std::stod(extractSecsFromUtcString(utcString));

    long long epochTime_us;

    utcToEpoch(epochTime_us, year, month, day, hour, min, secs);

    return epochTime_us;
}

std::string TimeUtil::extractYearFromUtcString(const std::string &utcString) {
    return utcString.substr(0, 4);
}

std::string TimeUtil::extractMonthFromUtcString(const std::string &utcString) {
    return utcString.substr(5, 2);
}

std::string TimeUtil::extractDayFromUtcString(const std::string &utcString) {
    return utcString.substr(8, 2);
}

std::string TimeUtil::extractHourFromUtcString(const std::string &utcString) {
    return utcString.substr(11, 2);
}

std::string TimeUtil::extractMinFromUtcString(const std::string &utcString) {
    return utcString.substr(14, 2);
}

std::string TimeUtil::extractSecsFromUtcString(const std::string &utcString) {
    return utcString.substr(17, 6);
}

std::string TimeUtil::extractTimeFromUtcString(const std::string &utcString) {
    return utcString.substr(11, 8);
}
