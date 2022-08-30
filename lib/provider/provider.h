/**
 * @file provider.h
 * @author Vida Wang (support@vida.wang)
 * @brief This file implemented a type of method to store pre-built functions and provide
 * interface to web.
 * 
 * 这个文件实现了一种方法来保存预先构建的功能和提供给前端的接口
 * 
 * @version 1.0.0
 * @date 2022-08-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include <Arduino.h>
#include <functional>
#include <arraybuffer.h>

typedef Elements *ProviderArguments;

typedef std::function<Element *(ProviderArguments arguments)> ProviderCallback;

typedef enum : uint8_t
{
    // default show to administrator
    // 默认显示给管理员
    PROVIDER_COMMON = 0b10000000,

    // indicate current provider action should question user or not
    // 指示执行当前provider是否是需要确认
    PROVIDER_QUESTION = 0b01000000,

    // only administrator could execute
    // 仅开放给管理员执行
    PROVIDER_ADMIN = 0b00100000,

    // default show to any user ( included administrator )
    // 指示当前provider显示给所有人
    PROVIDER_USER = 0b00010000,

    // both request and response should be encrypted
    // 执行provider时输入数据和返回数据是否需要加密
    PROVIDER_ENCRYPT = 0b00001000
} ProviderType;

class Provider
{
public:
    /**
     * @brief id of provider, automatically set by "createProvider"
     * user should not modify this id
     *
     * provider的ID，由createProvider函数设置，用户不应该修改此id
     */
    uint16_t id = 0;

    /**
     * @brief id set by user for other purpose
     * 用于其他用途的由用户设置的id
     */
    uint64_t customID = 0;

    /**
     * @brief name of provider, for human read
     * privoder的名字，给人看的
     */
    String name = "";

    /**
     * @brief provider callback
     * provider回调函数
     */
    ProviderCallback cb = nullptr;

    /**
     * @brief part of generate buffer
     * 用来生成buffer的一部分
     */
    uint8_t settings = 0x00;

    /**
     * @brief indicate data of current provider should encrypt or not
     * 指示当前provider的数据是否需要加密
     */
    bool encrypt = false;

    /**
     * @brief default empty arguments constructor
     * 默认无参构造
     */
    inline Provider() {}

    /**
     * @brief create a new provider
     * 创建一个新的provider
     *
     * @param id id set by createProvider 由createProvider自动填充的序列号
     * @param cb callback of provider, provider回调函数
     * @param name name of provider, for human read; provider的名字，给人看的
     * @param settings options of provider, provider的选项
     * @param lengthOfArguments length of arguments for js create inputs 给js用的创建参数输入框的参数长度
     */
    Provider(uint16_t id,
             ProviderCallback cb,
             String name,
             uint8_t settings,
             uint8_t lengthOfArguments = 0);

    inline ~Provider() {}

    /**
     * @brief create a buffer of current provider
     * 为当前provider创建buffer
     *
     * @param outLen output length 输出buffer的长度
     * @return uint8_t* buffer 缓存
     */
    uint8_t *getBuffer(uint64_t *outLen);
};
