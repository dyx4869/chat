#include "datetime.hpp"
#include <time.h>
DateTime::DateTime()
    : microSecondsSinceEpoch_(0)
{
}

DateTime::DateTime(int microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch)
{
}
DateTime DateTime::now() { return DateTime(time(NULL)); }
std::string DateTime::toDateString()
{
    char buf[512];
    tm* tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d-%02d-%02d", tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday);
    return buf;
}
std::string DateTime::toTimeString()
{
    char buf[512];
    tm* tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%02d:%02d:%02d", tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    return buf;
}

std::string DateTime::toDateTime()
{
    char buf[1024];
    tm* tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d-%02d-%02d %02d:%02d:%02d", tm_time->tm_year + 1900, tm_time->tm_mon + 1,
        tm_time->tm_mday, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    return std::string(buf);
}