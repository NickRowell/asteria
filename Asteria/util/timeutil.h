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
    static long long getEpochTimeShift();

    /**
     * @brief Converts the Unix epoch time (microseconds since 1970-01-01T00:00:00Z) to the equivalent UTC.
     *
     * @param epochTimeStamp_us
     *  The input epoch time to convert (microseconds after 1970-01-01T00:00:00Z)
     * @param year
     *  On exit, contains the years since AD 0
     * @param month
     *  On exit, contains the month of the year [1-12]
     * @param day
     *  On exit, contains the day of the month [1-31]
     * @param hour
     *  On exit, contains the hours since midnight [0-23]
     * @param min
     *  On exit, contains the minutes after the hour [0-59]
     * @param sec
     *  On exit, contains the seconds after the minute [0-61]. The range is generally 0-59; the extra range is to accommodate for leap seconds in certain systems.
     */
    static void epochToUtc(const long long &epochTimeStamp_us, int &year, int &month, int &day, int &hour, int &min, double &sec);

    /**
     * @brief Converts the UTC to the equivalent Unix epoch time (microseconds since 1970-01-01T00:00:00Z).
     *
     * @param epochTime_us
     *  On exit, containt the Unix epoch time (microseconds after 1970-01-01T00:00:00Z)
     * @param year
     *  Years since AD 0
     * @param month
     *  Month of the year [1-12]
     * @param day
     *  Day of the month [1-31]
     * @param hour
     *  Hours since midnight [0-23]
     * @param min
     *  Minutes after the hour [0-59]
     * @param sec
     *  Seconds after the minute [0-61]. The range is generally 0-59; the extra range is to accommodate for leap seconds in certain systems.
     */
    static void utcToEpoch(long long &epochTimeStamp_us, const int &year, const int &month, const int &day, const int &hour, const int &min, const double &sec);

    /**
     * @brief Converts the Unix epoch time (microseconds after 1970-01-01T00:00:00Z) to the equivalent
     * Julian day number. See https://stackoverflow.com/questions/466321/convert-unix-timestamp-to-julian
     *
     * @param epochTimeStamp_us
     *  The input epoch time to convert (microseconds after 1970-01-01T00:00:00Z)
     * @return
     *  The Julian Day Number
     */
    static double epochToJd(const long long &epochTimeStamp_us);

    /**
     * @brief Get the Greenwich Mean Sidereal Time for the given epoch time.
     *
     * Follows Example 3-5 from "Fundamentals of Astrodynamics and Applications", fourth edition.
     *
     * @param epochTimeStamp_us
     *  The input epoch time to convert (microseconds after 1970-01-01T00:00:00Z)
     * @return
     *  The Greenwich Mean Sidereal Time [decimal hours, 0-24]
     */
    static double epochToGmst(const long long &epochTimeStamp_us);

    /**
     * @brief Convert Greenwich Mean Sidereal Time to Local Sidereal Time.
     *
     * The Local Sidereal Time is equal to the Right Ascension of objects transiting the
     * observers meridean.
     *
     * @param gmst
     *  The current Greenwich Mean Sidereal Time [decimal hours]
     * @param longitude
     *  The longitude of the site, positive east [radians]
     * @return
     *  The Local Sidereal Time [decimal hours]
     */
    static double gmstToLst(const double &gmst, const double &longitude);

    /**
     * @brief TimeUtil::decimalHoursToHMS Convert a time in decimal hours [0-24] to HH:MM:SS.sss
     * @param dhour
     *  The input decimal hours [hours]
     * @param hour
     *  On exit, contains the whole number of hours [hours]
     * @param min
     *  On exit, contains the whole number of minutes [mins]
     * @param sec
     *  On exit, contains the seconds [sec]
     */
    static void decimalHoursToHMS(const double &dhour, int &hour, int &min, double &sec);

    /**
     * @brief TimeUtil::convertToUtcString
     * @param epochTimeStamp_us
     *  Microseconds after 1970-01-01T00:00:00Z
     *
     * NOTE: T is just a literal to separate the date from the time. Z means zero-hour offset (also 'Zulu time'), i.e. UTC.
     *
     * @return
     */
    static std::string epochToUtcString(const long long &epochTimeStamp_us);


    static long long utcStringToEpoch(const std::string &utcString);

    /**
     * @brief Extracts the year number from a string containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @param utc
     *  String containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @return
     *  The year number, i.e. YYYY.
     */
    static std::string extractYearFromUtcString(const std::string &utcString);

    /**
     * @brief Extracts the month number from a string containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @param utc
     *  String containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @return
     *  The month number, i.e. MM.
     */
    static std::string extractMonthFromUtcString(const std::string &utcString);

    /**
     * @brief Extracts the day number from a string containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @param utc
     *  String containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @return
     *  The day number, i.e. dd.
     */
    static std::string extractDayFromUtcString(const std::string &utcString);

    /**
     * @brief Extracts the hour number from a string containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @param utc
     *  String containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @return
     *  The hour number, i.e. hh.
     */
    static std::string extractHourFromUtcString(const std::string &utcString);

    /**
     * @brief Extracts the minute number from a string containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @param utc
     *  String containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @return
     *  The minute number, i.e. mm.
     */
    static std::string extractMinFromUtcString(const std::string &utcString);

    /**
     * @brief Extracts the seconds from a string containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @param utc
     *  String containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @return
     *  The seconds, i.e. ss.sss
     */
    static std::string extractSecsFromUtcString(const std::string &utcString);

    /**
     * @brief Extracts the time from a string containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @param utc
     *  String containing a UTC time in the format YYYY-MM-ddThh:mm:ss.sssZ
     * @return
     *  The time part, i.e. hh:mm:ss.
     */
    static std::string extractTimeFromUtcString(const std::string &utcString);
};

#endif // TIMEUTIL_H
