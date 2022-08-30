/**
 * @file esp32time.h
 * @author Vida Wang (support@vida.wang)
 * @brief This file provided functions related to time, reduced and modified
 * some functions from https://github.com/fbiego/ESP32Time, thanks to Felix Biego
 * 
 * 这个文件提供了时间相关的功能，从 https://github.com/fbiego/ESP32Time 去掉了一些
 * 不常用功能，对有些功能做出了一些修改，感谢 Felix Biego
 * 
 * @version 1.0.0
 * @date 2022-08-01
 *
 * @copyright Copyright (c) 2022
 *
 */

/*
   MIT License
  Copyright (c) 2021 Felix Biego
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#pragma once
#include <Arduino.h>
#include "time.h"
#include <sys/time.h>

class Esp32Time
{
private:
    struct tm getStructure();

public:
    /**
     * @brief set system time(unix epoch timestamp)
     * 设置系统时间(unix时间戳)
     *
     * @param time unix epoch timestamp, unix时间戳
     * @param timeZone time zone 时区
     */
    void setTime(uint64_t time, uint16_t timeZone = 8);

    /**
     * @brief set system time
     * 设置系统时间
     *
     * @param year year 年
     * @param month month 月
     * @param date date 日
     * @param hour hour 时
     * @param minute minute 分
     * @param second second 秒
     * @param ms millisecond 毫秒
     */
    void setTime(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second, uint16_t ms = 0);

    /**
     * @brief get formatted time string
     * 获取格式化的时间
     *
     * @param longFormat use full name or not 是否使用完整名称
     * @return String formatted time 格式化的时间
     */
    String getTimeFormat(bool longFormat = true);

    /**
     * @brief get day number in year
     * 获取在一年中的天数
     *
     * @return uint16_t number of day 天数
     */
    inline uint16_t getDayInYear()
    {
        return this->getStructure().tm_yday;
    }

    /**
     * @brief get milliseconds from unix epoch
     * 获取unix时间戳
     *
     * @return uint64_t unix epoch timestamp, unix时间戳
     */
    inline uint64_t getTime()
    {
        return ((uint64_t)((this->getSeconds() * 1000) + this->getMilli()));
    }

    /**
     * @brief get year
     * 获取年份
     *
     * @return uint16_t year 年份
     */
    inline uint16_t getYear()
    {
        return this->getStructure().tm_year + 1900;
    }

    /**
     * @brief get month
     * 获取月份
     *
     * @return uint8_t month 月份
     */
    inline uint8_t getMonth()
    {
        return this->getStructure().tm_mon;
    }

    /**
     * @brief get date(not included time)
     * 获取日期(不包括时间)
     *
     * @param longFormat use full name 是否使用完整名称
     * @return String formatted date 格式化的日期
     */
    String getDate(bool longFormat = true);

    /**
     * @brief get hour
     * 获取小时
     *
     * @param a24hour use a24-hour or not 是否使用24小时制
     * @return uint8_t hour 小时
     */
    inline uint8_t getHour(bool a24hour = true)
    {
        uint8_t hour = this->getStructure().tm_hour;
        return a24hour ? hour : (hour > 12 ? hour - 12 : hour);
    }

    /**
     * @brief get minute
     * 获取分钟
     *
     * @return uint8_t minute 分钟
     */
    inline uint8_t getMinute()
    {
        return this->getStructure().tm_min;
    }

    /**
     * @brief get second
     * 获取秒数
     *
     * @return uint8_t second 秒数
     */
    uint8_t getSecond()
    {
        return this->getStructure().tm_sec;
    }

    /**
     * @brief get seconds from unix epoch timestamp
     * 获取unix时间戳的秒数
     *
     * @return uint64_t seconds from unix epoch timestamp, unix时间戳的秒数
     */
    uint64_t getSeconds();

    /**
     * @brief get milliseconds
     * 获取毫秒数
     *
     * @return uint16_t milliseconds 毫秒数
     */
    uint16_t getMilli();

    /**
     * @brief get microseconds
     * 获取微秒数
     *
     * @return uint32_t microseconds 微秒数
     */
    uint32_t getMicro();

    /**
     * @brief get day in week
     * 获取在星期内的天数
     *
     * @return uint8_t day 天数
     */
    inline uint8_t getDay()
    {
        return this->getStructure().tm_wday;
    }

    /**
     * @brief get formatted time string
     * 获取格式化的时间
     *
     * @param longFormat use full name or not 是否使用完整名称
     * @return String formatted time 格式化的时间
     */
    inline String getDateTime(bool longFormat = true)
    {
        return this->getTimeFormat(longFormat);
    }

    inline Esp32Time() {}
    inline ~Esp32Time() {}
};

extern Esp32Time *globalTime;