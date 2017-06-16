#include "timeutil.h"

#include <sstream>              // ostringstream

TimeUtil::TimeUtil()
{

}

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
 * @brief TimeUtil::convertToUtcString
 * @param epochTimeStamp_us
 *  Microseconds after 1970-01-01T00:00:00Z
 *
 * NOTE: T is just a literal to separate the date from the time. Z means zero-hour offset (also 'Zulu time'), i.e. UTC.
 *
 * @return
 */
std::string TimeUtil::convertToUtcString(long long epochTimeStamp_us) {

    // Split into whole seconds and remainder microseconds
    long long epochTimeStamp_s = epochTimeStamp_us / 1000000LL;
    long long epochTimeStamp_us_remainder = epochTimeStamp_us % 1000000LL;
    int epochTimeStamp_ms_remainder = epochTimeStamp_us_remainder / 1000;

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
    // months since January	[0-11]; convert to 1-12
    int tm_mon = ptm->tm_mon + 1;
    // years since 1900; convert to years since AD 0
    int tm_year = ptm->tm_year + 1900;

    // Construct date string
    std::ostringstream strs;

    // Zero-padded month
    char monArr [10];
    sprintf (monArr, "%02d", tm_mon);
    std::string monStr(monArr);

    // Zero-padded day
    char dayArr [10];
    sprintf (dayArr, "%02d", tm_mday);
    std::string dayStr(dayArr);

    // Zero-padded hour
    char hourArr [10];
    sprintf (hourArr, "%02d", tm_hour);
    std::string hourStr(hourArr);

    // Zero-padded minute
    char minArr [10];
    sprintf (minArr, "%02d", tm_min);
    std::string minStr(minArr);

    // Zero-padded second
    char secArr [10];
    sprintf (secArr, "%02d", tm_sec);
    std::string secStr(secArr);

    // Zero-padded microseconds remainder
    char usecArr [10];
    sprintf (usecArr, "%03d", epochTimeStamp_ms_remainder);
    std::string usecStr(usecArr);

    strs << tm_year << "-" << monStr << "-" << dayStr << "T" << hourStr << ":" << minStr << ":" << secStr << "." << usecStr << "Z";

    return strs.str();
}

/**
 * 2017-05-16T16:26:32.155Z
 *
 * @brief TimeUtil::extractYearFromUtcString
 * @param utc
 * @return
 */
std::string TimeUtil::extractYearFromUtcString(std::string utc) {
    return utc.substr(0, 4);
}

std::string TimeUtil::extractMonthFromUtcString(std::string utc) {
    return utc.substr(5, 2);
}

std::string TimeUtil::extractDayFromUtcString(std::string utc) {
    return utc.substr(8, 2);
}

std::string TimeUtil::extractTimeFromUtcString(std::string utc) {
    return utc.substr(11, 8);
}

const std::regex TimeUtil::getUtcRegex() {
    return std::regex("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}Z");
}

const std::regex TimeUtil::getTimeRegex() {
    return std::regex("[0-9]{2}:[0-9]{2}:[0-9]{2}");
}
