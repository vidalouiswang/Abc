/**
 * @file mycrypto.h
 * @author Vida Wang (support@vida.wang)
 * @brief This file provided functions related to encode, encrypt.
 * SHA and AES part use ESP32 hardware acceleration to implemented;
 * ESPRESSIF ESP32 technical reference manual AES and SHA part.
 * Base64 encode and decode implemented by software.
 *
 * 这个文件提供了编码、加密相关的功能。
 * SHA和AES使用ESP32硬件加速器实现。
 * 乐鑫ESP32技术参考手册AES和SHA部分。
 * Base64编解码使用软件实现。
 * @version 1.0.5
 * @date 2022-08-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MY_CRYPTO_H_
#define MY_CRYPTO_H_

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "soc/dport_access.h"
#include "soc/hwcrypto_reg.h"

#if CONFIG_IDF_TARGET_ESP32S3
#include "esp_crypto_lock.h"
#endif

#ifndef _DRIVER_PERIPH_CTRL_H_
#if ESP_IDF_VERSION_MAJOR < 4
#include "esp_private/periph_ctrl.h"
#else
#include "driver/periph_ctrl.h"
#endif
#endif


#define TO_32_LE(x) (                         \
    ((uint32_t)(((uint8_t *)(x))[0])) |       \
    ((uint32_t)(((uint8_t *)(x))[1]) << 8) |  \
    ((uint32_t)(((uint8_t *)(x))[2]) << 16) | \
    ((uint32_t)(((uint8_t *)(x))[3]) << 24))

#define MY_CRYPTO_DEBUG_HEADER "mycrypto"

namespace mycrypto
{
    typedef enum
    {
        SHA1 = 1,
        SHA256 = 0
    } SHAType;

    typedef enum
    {
        LOWER_CASE,
        UPPER_CASE
    } SHAOutputCase;

    class SHA
    {
    private:
        /**
         * @brief get sha digest
         * 获取sha数字摘要
         *
         * @param data input data 输入数据
         * @param length length of input data 输入数据的长度
         * @param output sha digest, sha数字摘要
         * @param type type of sha digest 数字摘要的类型
         */
        static void sha(uint8_t *data, uint64_t length, uint32_t *output, SHAType type = SHA256);

        /**
         * @brief get sha digest arduino String
         * 获取arduino String 类型的sha数字摘要
         *
         * @param data input data 输入数据
         * @param length length of input data 输入数据的长度
         * @param type type of sha digest 数字摘要的类型
         * @param hexCase format of hex string 16进制字符串的格式
         * @return String sha digest in hex string format 16进制字符串的sha数字摘要
         */
        static String aSHA(uint8_t *data, uint64_t length, SHAType type, SHAOutputCase hexCase = LOWER_CASE);

        /**
         * @brief get uint8 array sha digest
         * 获取uint8 数组的sha数字摘要
         *
         * @param data input data 输入数据
         * @param length length of input data 输入数据的长度
         * @param output sha digest, sha数字摘要
         * @param type type of sha digest 数字摘要的类型
         */
        static void convertU32ToU8(uint8_t *data, uint64_t length, uint8_t *output, SHAType type);

    public:
        /**
         * @brief enable esp32 sha hardware acceleration, attention atteched
         * 使能esp32 sha加速器, 请阅读注意事项
         *
         * @attention must call this function before any functions that get sha digest
         * 必须在调用其他获取sha数字摘要的函数之前调用此函数
         */
        static inline void initialize()
        {
            periph_module_enable(PERIPH_SHA_MODULE);
        }

        /**
         * @brief get sha1
         * 获取sha1
         *
         * @param data input data 输入数据
         * @param length length of input 输入数据的长度
         * @param output uint32 array sha digest, uint32 数组sha数字摘要
         */
        static inline void sha1(uint8_t *data, uint64_t length, uint32_t *output)
        {
            sha(data, length, output, SHA1);
        }

        /**
         * @brief get sha1
         * 获取sha1
         *
         * @param data input data 输入数据
         * @param length length of input 输入数据的长度
         * @param output uint8 array sha digest, uint8 数组sha数字摘要
         */
        static inline void sha1(uint8_t *data, uint64_t length, uint8_t *output)
        {
            convertU32ToU8(data, length, output, SHA1);
        }

        /**
         * @brief get sha1
         * 获取sha1
         *
         * @param data input data, uint32 输入数据，uint32
         * @param output uint8 array sha digest, uint8 数组sha数字摘要
         */
        static inline void sha1(uint32_t data, uint8_t *output)
        {
            uint8_t t[4] = {((uint8_t)(data >> 24)), ((uint8_t)(data >> 16)), ((uint8_t)(data >> 8)), ((uint8_t)(data))};
            convertU32ToU8(t, 4, output, SHA1);
        }

        /**
         * @brief get sha1
         * 获取sha1
         *
         * @param data input data 输入数据
         * @param length length of input 输入数据的长度
         * @param hexCase format of hex string 16进制字符串的格式
         * @return String sha digest in arduino String hex format, arduino String类对象格式的16进制字符串
         */
        static inline String sha1(uint8_t *data, uint64_t length, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA(data, length, SHA1, hexCase);
        }

        /**
         * @brief get sha1
         * 获取sha1
         *
         * @param data arduino String object, arduino String 类对象
         * @param hexCase format of hex string 16进制字符串的格式
         * @return String sha digest in arduino String hex format, arduino String类对象格式的16进制字符串
         */
        static inline String sha1(String data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data.c_str(), data.length(), SHA1, hexCase);
        }

        /**
         * @brief get sha1
         * 获取sha1
         *
         * @param data pointer of arduino String object, arduino String 类对象的指针
         * @param hexCase format of hex string 16进制字符串的格式
         * @return String sha digest in arduino String hex format, arduino String类对象格式的16进制字符串
         */
        static inline String sha1(String *data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data->c_str(), data->length(), SHA1, hexCase);
        }

        /**
         * @brief get sha1
         * 获取sha1
         *
         * @param data c string, c字符串
         * @param hexCase format of hex string 16进制字符串的格式
         * @return String sha digest in arduino String hex format, arduino String类对象格式的16进制字符串
         */
        static inline String sha1(const char *data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data, strlen(data), SHA1, hexCase);
        }

        /**
         * @brief get sha256
         * 获取sha256
         *
         * @param data input 输入
         * @param length length of input 输入数据的长度
         * @param output uint8 array sha digest, uint8 数组sha数字摘要
         */
        static inline void sha256(uint8_t *data, uint64_t length, uint8_t *output)
        {
            convertU32ToU8(data, length, output, SHA256);
        }

        /**
         * @brief get sha256
         * 获取sha256
         *
         * @param data input 输入
         * @param length length of input 输入数据的长度
         * @param output uint32 array sha digest, uint32 数组sha数字摘要
         */
        static inline void sha256(uint8_t *data, uint64_t length, uint32_t *output)
        {
            sha(data, length, output, SHA256);
        }

        /**
         * @brief get sha256
         * 获取sha256
         *
         * @param data input data, uint32 输入数据，uint32
         * @param output uint8 array sha digest, uint8 数组sha数字摘要
         */
        static inline void sha256(uint32_t data, uint8_t *output)
        {
            uint8_t t[4] = {((uint8_t)(data >> 24)), ((uint8_t)(data >> 16)), ((uint8_t)(data >> 8)), ((uint8_t)(data))};
            convertU32ToU8(t, 4, output, SHA256);
        }

        /**
         * @brief get sha256
         * 获取sha256
         *
         * @param data input 输入
         * @param length length of input 输入数据的长度
         * @param hexCase format of hex string 16进制字符串的格式
         * @return String sha digest in arduino String hex format, arduino String类对象格式的16进制字符串
         */
        static inline String sha256(uint8_t *data, uint64_t length, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA(data, length, SHA256, hexCase);
        }

        /**
         * @brief get sha256
         * 获取sha256
         *
         * @param data arduino String object, arduino String 类对象
         * @param hexCase format of hex string 16进制字符串的格式
         * @return String sha digest in arduino String hex format, arduino String类对象格式的16进制字符串
         */
        static inline String sha256(String data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data.c_str(), data.length(), SHA256, hexCase);
        }

        /**
         * @brief generate sha256 using c string
         * 用c字符串生成sha256
         *
         * @param data input string 输入字符串
         * @param output output 输出
         */
        static inline void sha256(const char *data, uint8_t *output)
        {
            if (!data)
                return;
            if (!strlen(data))
                return;

            bzero(output, 32);
            convertU32ToU8((uint8_t *)data, strlen(data), output, SHA256);
        }

        /**
         * @brief get sha256
         * 获取sha256
         *
         * @param data pointer of arduino String object, arduino String 类对象的指针
         * @param hexCase format of hex string 16进制字符串的格式
         * @return String sha digest in arduino String hex format, arduino String类对象格式的16进制字符串
         */
        static inline String sha256(String *data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data->c_str(), data->length(), SHA256, hexCase);
        }

        /**
         * @brief get sha256
         * 获取sha256
         *
         * @param data c string, c字符串
         * @param hexCase format of hex string 16进制字符串的格式
         * @return String sha digest in arduino String hex format, arduino String类对象格式的16进制字符串
         */
        static inline String sha256(const char *data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data, strlen(data), SHA256, hexCase);
        }
    };

    class Base64
    {
    private:
        static uint8_t getCharIndex(uint8_t c);
        static uint8_t *base64Decode(uint8_t *data, uint64_t iLen, uint64_t *oLen);

    public:
        /**
         * @brief base64 encode
         * base64编码
         *
         * @param data input 输入
         * @param length length of input 输入数据的长度
         * @return char* string with '\0',  '\0'结尾的字符串
         */
        static char *base64Encode(uint8_t *data, uint64_t length);

        /**
         * @brief base64 encode
         * base64编码
         *
         * @param data c string, c字符串
         * @return String arduino String object, arduino String类对象
         */
        static inline String base64Encode(const char *data)
        {
            if (!data)
                return "";

            char *a = base64Encode((uint8_t *)data, strlen(data));
            String b = String(a); // this will make a copy in RAM
            delete a;
            return b;
        }

        /**
         * @brief base64 encode
         * base64编码
         *
         * @param data String arduino String object, arduino String类对象
         * @return String String arduino String object, arduino String类对象
         */
        static inline String base64Encode(String data)
        {
            return base64Encode((const char *)data.c_str());
        }

        /**
         * @brief base64 decode
         * base64解码
         *
         * @param data String arduino String object, arduino String类对象
         * @param oLen length of output 输出数据的长度
         * @return uint8_t* output 输出
         */
        static inline uint8_t *base64Decode(String data, uint64_t *oLen)
        {
            return base64Decode((uint8_t *)data.c_str(), data.length(), oLen);
        }

        /**
         * @brief base64 decode
         * base64解码
         *
         * @param data String arduino String object, arduino String类对象
         * @return String String arduino String object, arduino String类对象
         */
        static inline String base64Decode(String data)
        {
            uint64_t oLen = 0;
            uint8_t *output = base64Decode((uint8_t *)data.c_str(), data.length(), &oLen);
            String a = String((char *)output); // this will make a copy
            delete output;
            return a;
        }

        /**
         * @brief base64 decode
         * base64解码
         *
         * @param data c string, c 字符串
         * @param oLen length of output, 输出数据的长度
         * @return uint8_t* output 输出
         */
        static inline uint8_t *base64Decode(const char *data, uint64_t *oLen)
        {
            if (!data)
                return nullptr;
            int length = strlen(data);
            return base64Decode((uint8_t *)data, length, oLen);
        }
    };

    // AES encryption and decryption according to ESPRESSIF ESP32 technical reference manual,
    // chapter 22, page 523(Chinese version)
    class AES
    {
    public:
        static inline void initialize() { periph_module_enable(PERIPH_AES_MODULE); }

        /**
         * @brief aes encryption, cbc mode, 256 bits key
         * aes加密，cbc模式, 256位密匙长度
         *
         * @param key 32 bytes key, 32字节密匙
         * @param iv 16 bytes iv, 16字节初始化向量
         * @param plain input data, 输入数据
         * @param length length of input, 输入数据的长度
         * @param outLen length of output, 输出数据的长度
         * @return uint8_t* output 输出
         */
        static uint8_t *aes256CBCEncrypt(const uint8_t *key,
                                         const uint8_t *iv,
                                         const uint8_t *plain,
                                         uint32_t length,
                                         uint32_t *outLen);

        /**
         * @brief aes decryption, cbc mode 256 bits
         * aes解密，cbc模式，256位密匙长度
         *
         * @param key 32 bytes key, 32字节密匙
         * @param iv 16 bytes iv, 16字节初始化向量
         * @param cipher input data, 输入数据
         * @param length length of input, 输入数据的长度
         * @param outLen length of output, 输出数据的长度
         * @return uint8_t* output 输出
         */
        static uint8_t *aes256CBCDecrypt(
            const uint8_t *key,
            const uint8_t *iv,
            const uint8_t *cipher,
            uint32_t length,
            uint32_t *outLen);

        /**
         * @brief aes encryption, cbc mode 256 bits
         * aes加密，cbc模式，256位密匙长度
         *
         * @param key arduino String object, 32 bytes key; arduino String类对象，32字节密匙
         * @param iv arduino String object, 16 bytes iv; arduino String类对象，16字节初始化向量
         * @param plain input data, 输入数据
         * @return String arduino String object output, hex format; arduino String类对象输出，16进制字符串
         */
        static String aes256CBCEncrypt(String key,
                                       String iv,
                                       String plain);

        /**
         * @brief aes decryption, cbc mode 256 bits
         * aes解密，cbc模式，256位密匙长度
         *
         * @param key arduino String object, 32 bytes key; arduino String类对象，32字节密匙
         * @param iv arduino String object, 16 bytes iv; arduino String类对象，16字节初始化向量
         * @param plain input data, 输入数据
         * @return String arduino String object output, hex format; arduino String类对象输出，16进制字符串
         */
        static String aes256CBCDecrypt(
            String key,
            String iv,
            String cipher);
    };
}

#endif