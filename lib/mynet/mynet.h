/**
 * @file mynet.h
 * @author Vida Wang (support@vida.wang)
 * @brief This file packed soft ap method.
 * 这个文件把soft ap函数包装了一下
 *
 * @version 1.0.0
 * @date 2022-08-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MYNET_H_
#define MYNET_H_

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

class MyNet
{
private:
    IPAddress *apIP = nullptr;

public:
    /**
     * @brief start soft access point
     * 开启热点
     *
     * @param ssid service set identifier 热点名称
     * @param apIP default ap gateway 默认ap网关
     * @param pwd password 密码
     * @return true success 成功
     * @return false failed 失败
     */
    bool startAP(const char *ssid, IPAddress *apIP, const char *pwd = nullptr);
};
extern MyNet myNet;

#endif