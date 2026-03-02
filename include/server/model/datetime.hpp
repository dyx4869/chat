#ifndef DATETIME_H
#define DATETIME_H
#include <string>
#include <iostream>
#include <cstdint>

class DateTime
{
private:
    int64_t microSecondsSinceEpoch_;

public:
    DateTime();
    explicit DateTime(int microSecondsSinceEpoch_);
    static DateTime now();
    std::string toDateString();
    std::string toTimeString();
    std::string toDateTime();
};

#endif