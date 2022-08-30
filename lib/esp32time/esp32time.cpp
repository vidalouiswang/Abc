#include "esp32time.h"

String Esp32Time::getTimeFormat(bool longFormat)
{
    struct tm time = this->getStructure();
    char s[61];
    if (longFormat)
    {
        strftime(s, 60, "%H:%M:%S %A, %B %d %Y", &time);
    }
    else
    {
        strftime(s, 60, "%H:%M:%S %a, %b %d %Y", &time);
    }
    return String(s);
}

struct tm Esp32Time::getStructure()
{
    time_t now;
    struct tm tms;
    time(&now);
    localtime_r(&now, &tms);
    return tms;
}

void Esp32Time::setTime(uint64_t time, uint16_t timeZone)
{
    long second = time / 1000;
    int ms = time % 1000;

    struct timeval tv;
    tv.tv_sec = second;
    tv.tv_usec = ms;

    setenv("TZ", String(String("GMT-") + String(timeZone)).c_str(), 1);
    tzset();

    settimeofday(&tv, NULL);
}

void Esp32Time::setTime(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second, uint16_t ms)
{
    struct tm time = {0};
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = date;
    time.tm_hour = hour;
    time.tm_min = minute;
    time.tm_sec = second;
    time_t t = mktime(&time);
    setTime((t * 1000) + ms);
}

String Esp32Time::getDate(bool longFormat)
{
    struct tm time = this->getStructure();

    char buf[61];
    buf[60] = 0;

    if (longFormat)
    {
        strftime(buf, 60, "%A, %B %d %Y", &time);
    }
    else
    {
        strftime(buf, 60, "%a, %b %d %Y", &time);
    }

    return String(buf);
}

uint64_t Esp32Time::getSeconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

uint16_t Esp32Time::getMilli()
{
    return this->getMicro() / 1000;
}

uint32_t Esp32Time::getMicro()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
}

Esp32Time *globalTime = new Esp32Time();