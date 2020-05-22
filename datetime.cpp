/* A small library for date and time support, giving some basic methods from
 * std::chrono and some additional tools.
 *
 * Defines the namespace DateTime containing:
 *  clock
 *  time_point
 *  duration
 *  seconds
 *  minutes
 *  hours
 *  days
 *  weeks
 *  months
 *  years
 *  now
 *  parseDateStr
 *  parseDurationStr
 *  to_time_t
 *  to_tm
 *  toString
 *  toDateString
 *  toClockTimeStr
 *  getBeginOfDay
 *  getLastMonday
 *  getLastFirstOfMonth
 *  getLastFirstOfYear
 */

#include <chrono>     // c++ time and date
#include <ctype.h>      // c single character operations
#include <ctime>      // c date & time objects
#include <iomanip>    // c date & time functions
#include <string>     // strings
#include <sstream>    // string stream
#include <exception>  // exceptions


#include <iostream>


namespace DateTime {

    // Make some default types available

    namespace chrono = std::chrono;
    using clock = chrono::system_clock;
    using time_point = chrono::time_point<clock>;
    using duration = clock::duration;

    using seconds = chrono::seconds;
    using minutes = chrono::minutes;
    using hours = chrono::hours;
    typedef chrono::duration<int, std::ratio<       24*3600,1>> days;
    typedef chrono::duration<int, std::ratio<     7*24*3600,1>> weeks;
    typedef chrono::duration<int, std::ratio<    30*24*3600,1>> months;
    typedef chrono::duration<int, std::ratio<365*30*24*3600,1>> years;


    // Define constants

    // dd.mm.yyyy hh:mm:ss
    const char* DATEFORMAT = (char*) "%d.%m.%Y %H:%M:%S";
    const int DATESIZE = 19;

    /* This exception is thrown by parseDateStr() if the given string does not
     * match the specified format. */
    class DateFormatException : public std::exception {};

    /* The current time. */
    time_point now() {
        return clock::now();
    }

    /* Get a time point from a string. */
    time_point parseDateStr(const std::string& s) {
        std::tm tm{0};
        // Ignore daylight saving time. This fixes a but where one hour was
        // added to some times, but is probably not consistent for all times.
        tm.tm_isdst = -1;
        std::istringstream stream(s);
        stream >> std::get_time(&tm, DATEFORMAT);
        if (stream.fail()) {
            throw DateFormatException();
        }
        std::time_t time_t = std::mktime(&tm);
        return chrono::system_clock::from_time_t(time_t);
    }

    /* Get a duration from a string. */
    duration parseDurationStr(const std::string& str) {
        try {
            std::string::size_type number_length;
            int number = std::stoi(str, &number_length);
            std::string unit = str.substr(number_length);
            if (unit.compare("m") == 0) {
                return minutes(number);
            } else if (unit.compare("h") == 0) {
                return hours(number);
            } else if (unit.compare("d") == 0) {
                return days(number);
            } else {
                throw DateFormatException();
            }
        } catch(std::exception& ex) {
            throw DateFormatException();
        }
    }

    /* Tranlate to a time_t object. */
    std::time_t to_time_t(const time_point& time) {
        return chrono::system_clock::to_time_t(time);
    }

    /* Tranlate to a tm object. */
    std::tm *to_tm(const time_point& time) {
        std::time_t time_t = to_time_t(time);
        return localtime(&time_t);
    }

    /* Convert a date to a string. */
    std::string toString(const time_point& time) {
        // one byte more for null termination
        char buff[DATESIZE + 1];
        std::tm *tm = to_tm(time);
        std::strftime(buff, DATESIZE + 1, DATEFORMAT, tm);
        return std::string(buff);
    }

    /* Convert a date to a string skipping the clock time. */
    std::string toDateString(const time_point& time) {
        // e.g. 'Mon 01.01.1970'
        char buff[16];
        std::tm *tm = to_tm(time);
        std::strftime(buff, 16, "%a %d.%m.%Y", tm);
        return std::string(buff);
    }

    /* Convert a date to a string using only the clock time. */
    std::string toClockTimeStr(const time_point& time) {
        // e.g. '17:21:02'
        char buff[9];
        std::tm *tm = to_tm(time);
        std::strftime(buff, 9, "%H:%M:%S", tm);
        return std::string(buff);
    }

    /* Convert a duration to a string. */
    std::string toString(duration time) {
        int h = chrono::duration_cast<hours>(time).count();
        time %= hours(1);
        int m = chrono::duration_cast<minutes>(time).count();
        std::stringstream ss;
        if (h>0) {
            ss << h << "h";
        }
        if (m>0) {
            ss << m << "min";
        }
        // ensure that no empty string is returned
        if (ss.str().empty()) {
            ss << "0";
        }
        return ss.str();
    }

    /* Get the time 0:00 of the given day. */
    time_point getBeginOfDay(const time_point& time) {
        std::tm *tm = to_tm(time);
        time_point res = time;
        res -= hours( tm->tm_hour );
        res -= minutes( tm->tm_min );
        res -= seconds( tm->tm_sec );
        return res;
    }

    /* Get the first day of the week of the given day. */
    time_point getLastMonday(const time_point& time) {
        std::tm *tm = to_tm(time);
        time_point res = time;
        res -= days( (tm->tm_wday - 1) % 7 );
        return res;
    }

    /* Get the first day of the month of the given day. */
    time_point getLastFirstOfMonth(const time_point& time) {
        std::tm *tm = to_tm(time);
        time_point res = time;
        res -= days( tm->tm_mday - 1 );
        return res;
    }

    /* Get the first day of the year of the given day. */
    time_point getLastFirstOfYear(const time_point& time) {
        std::tm *tm = to_tm(time);
        time_point res = time;
        res -= days( tm->tm_yday - 1 );
        return res;
    };
    
}
