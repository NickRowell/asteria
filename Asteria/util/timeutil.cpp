#include "timeutil.h"

#include <sstream>              // ostringstream

TimeUtil::TimeUtil() {

}

// Format spec for string representation of UTC
const char * TimeUtil::utcFormatString = "%04d-%02d-%02dT%02d:%02d:%06.3fZ";

// Regex suitable for identifying strings containing e.g. 19:41:09
const std::regex TimeUtil::timeRegex("[0-9]{2}:[0-9]{2}:[0-9]{2}");

// Regex suitable for identifying strings containing e.g. 2017-06-14T19:41:09.282Z
const std::regex TimeUtil::utcRegex("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}Z");

/**
 * Get the time difference in microseconds between the clock time (records time since bootup)
 * and the epoch time (i.e. current time of day). This is useful for referencing the recorded time of video frames
 * to UTC. Note that this must be recomputed whenever the computer hibernates.
 *
 * See http://stackoverflow.com/questions/10266451/where-does-v4l2-buffer-timestamp-value-starts-counting
 *
 * \return The time difference in microseconds between the clock time (records time since bootup)
 * and the current time of day.
 */
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

long long TimeUtil::getUpTime() {

    // Records time since bootup, to nanosecond accuracy
    struct timespec  vsTime;

    clock_gettime(CLOCK_MONOTONIC, &vsTime);

    long long uptime_us = vsTime.tv_sec* 1000000LL + (long long)  round( vsTime.tv_nsec/ 1000.0);
    return uptime_us;
}

/**
 * @brief TimeUtil::convertEpochToUtc
 *
 * @param epochTimeStamp_us
 *  The input epoch time to convert (microseconds after 1970-01-01T00:00:00Z)
 * @param year
 *  On exit, contains the years since 1900
 * @param month
 *  On exit, contains the months since January [0-11]
 * @param day
 *  On exit, contains the day of the month [1-31]
 * @param hour
 *  On exit, contains the hours since midnight [0-23]
 * @param min
 *  On exit, contains the minutes after the hour [0-59]
 * @param sec
 *  On exit, contains the seconds after the minute [0-61]. The range is generally 0-59; the extra range is to accommodate for leap seconds in certain systems.
 */
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

    return;
}

/**
 * @brief TimeUtil::convertEpochToJd
 *  Converts the Unix epoch time (microseconds after 1970-01-01T00:00:00Z) to the equivalent
 * Julian day number. See https://stackoverflow.com/questions/466321/convert-unix-timestamp-to-julian
 *
 * @param epochTimeStamp_us
 *  The input epoch time to convert (microseconds after 1970-01-01T00:00:00Z)
 * @return
 *  The Julian Day Number
 */
double TimeUtil::epochToJd(const long long &epochTimeStamp_us) {
    // The Unix epoch (zero-point) is January 1, 1970 GMT. That corresponds to the Julian day of 2440587.5
    // We just need to add the number of consecutive days since then, ignoring leap seconds.
    // The constant 86400000000.0 is the number of microseconds in a day.
    return 2440587.5 + epochTimeStamp_us/86400000000.0;
}

/**
 * @brief TimeUtil::epochToGmst
 * Get the Greenwich Mean Sidereal Time for the given epoch time.
 *
 * See http://www.cv.nrao.edu/~rfisher/Ephemerides/times.html
 *
 * @param epochTimeStamp_us
 *  The input epoch time to convert (microseconds after 1970-01-01T00:00:00Z)
 * @return
 *  The Greenwich Mean Sidereal Time [units?]
 */
double TimeUtil::epochToGmst(const long long &epochTimeStamp_us) {

    // Julian day number
    double jd = epochToJd(epochTimeStamp_us);
    // Julian days since 2000 Jan. 1 12h UT1
    double d = jd - 2451545.0;
    // Julian centuries since 2000 Jan. 1 12h UT1
    double t = d / 36525.0;
    // TODO: what are the units of GMST? "Seconds at UT1 = 0"
    double gmst = 24110.54841 + (8640184.812866 * t) + (0.093104 * t * t) - (0.0000062 * t * t * t);

    // Get the seconds since UT = 0 for today
    int year, month, day, hour, min;
    double sec;
    epochToUtc(epochTimeStamp_us, year, month, day, hour, min, sec);
    double secsSinceUt0 = hour * 3600 + min * 60 + sec;

    // Add this to the gmst to get the current gmst
    gmst += secsSinceUt0;

    // Now convert this to time within the current day

    // Convert to days:
    double gmst_d = gmst / (60.0 * 60.0 * 24.0);
    // Get remainder (fraction of the day)
    gmst_d -= std::floor(gmst_d);
    // Convert to hours
    double gmst_h = gmst_d * 24;
    gmst_h -= std::floor(gmst_h);
    double gmst_m = (gmst_h - std::floor(gmst_h)) * 60.0;
    double gmst_s = (gmst_m - std::floor(gmst_m)) * 60.0;

    fprintf(stderr, "GMST = %f\t%02.0f:%02.0f:%02.0f\n", gmst, gmst_h, gmst_m, gmst_s);

    return gmst;
}

/**
 * @brief TimeUtil::convertToUtcString
 * @param epochTimeStamp_us
 *  Microseconds after 1970-01-01T00:00:00Z
 *
 * NOTE: T is just a literal to separate the date from the time. Z means zero-hour offset (also 'Zulu time'), i.e. UTC.
 *
 * @return
 */
std::string TimeUtil::epochToUtcString(const long long &epochTimeStamp_us) {

    int year, month, day, hour, min;
    double sec;
    epochToUtc(epochTimeStamp_us, year, month, day, hour, min, sec);

    // Convert to years since AD 0
    year += 1900;
    // Convert months to 1-12
    month += 1;

    // Construct date string
    char utcStr[100];
    sprintf (utcStr, utcFormatString, year, month, day, hour, min, sec);

    std::string utcStrr(utcStr);
    return utcStrr;
}

/**
 * 2017-05-16T16:26:32.155Z
 *
 * @brief TimeUtil::extractYearFromUtcString
 * @param utc
 * @return
 */
std::string TimeUtil::extractYearFromUtcString(const std::string &utc) {
    return utc.substr(0, 4);
}

std::string TimeUtil::extractMonthFromUtcString(const std::string &utc) {
    return utc.substr(5, 2);
}

std::string TimeUtil::extractDayFromUtcString(const std::string &utc) {
    return utc.substr(8, 2);
}

std::string TimeUtil::extractTimeFromUtcString(const std::string &utc) {
    return utc.substr(11, 8);
}
