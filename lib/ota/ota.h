/**
 * @file ota.h
 * @author Vida Wang (support@vida.wang)
 * @brief This file implemented OTA update function using websocket client.
 * 这个文件实现了使用websocket执行OTA更新的功能
 *
 * @version 1.0.0
 * @date 2022-08-16
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OTA_H_
#define OTA_H_
#include <Arduino.h>
#include <mywebsocket.h>
#include "esp_ota_ops.h"
#include <arraybuffer.hpp>
#include <config.h>
#include <functional>
#include <vector>
#include <esp32time.h>

#define OTA_DEBUG_HEADER "OTAUpdate"

using namespace myWebSocket;

typedef std::function<void(int)> OTACallback;

class WebsocketOTA
{
private:
    // ota update handle
    // ota 升级句柄
    esp_ota_handle_t handle = 0;

    // ota data block index
    // ota 数据块索引
    uint32_t index = 0;

    // ota update partition
    // ota升级分区
    const esp_partition_t *partition;

    // for request ota block from server
    // 用于向服务器请求ota数据块
    std::vector<Element *> *request = nullptr;

    // callback for ota update aborted
    // ota升级中断的回调函数
    OTACallback abortCallback = nullptr;

    // callback for ota update started
    // ota升级开始的回调函数
    OTACallback startCallback = nullptr;

    // length of firmware
    // 固件长度
    uint32_t firmwareLength = 0;

    // length of single ota data block
    // 单个ota数据块长度
    uint32_t blockLength = 0;

    // for request ota data block from server
    // 用于向服务器请求ota数据块
    uint64_t bufferOutLen = 0;

    // offset of data written to ota partition
    // 向ota分区写入的偏移量
    uint64_t writeOffset = 0;

    /**
     * @brief request next ota block or resend last request
     * 请求下一个ota数据块或重复上次请求
     */
    void fetchNext();

    /**
     * @brief clear ota data block buffer
     * 清除ota数据块缓存
     *
     * @param output ota data block from server 服务器发来的ota数据块
     */
    void clearBuffer(std::vector<Element *> *output);

public:
    // websocket client to server for ota update
    // 用于ota升级的websocket客户端
    WebSocketClient *client = nullptr;

    inline WebsocketOTA() {}

    // indicate if ota update ready
    // 指示ota升级是否准备就绪
    bool otaInitialized = false;

    // indicate if ota update has been aborted
    // 指示ota升级是否被中断
    bool aborted = false;

    /**
     * @brief default websocket ota constructor
     * 默认websocket ota升级构造函数
     *
     * @param startOTARequest websocket ota request sent from web page 由前端发来的ota升级请求
     * @param startCallback callback for ota update started 开始ota升级的回调函数
     * @param abortCallback callback for ota update aborted, ota升级中断的回调函数
     */
    WebsocketOTA(std::vector<Element *> *startOTARequest, OTACallback startCallback, OTACallback abortCallback);

    /**
     * @brief start ota update process
     * 开始ota升级
     *
     * @param universalID id of current device 当前设备id
     * @param domain domain for websocket ota update, websocket升级服务器域名
     * @param port port for websocket ota update, websocket升级服务器端口
     * @param path path for websocket ota update, websocket升级服务器路径
     */
    void start(Element *universalID, String domain, uint16_t port, String path);

    ~WebsocketOTA();
};

#endif