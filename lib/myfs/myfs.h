/**
 * @file myfs.h
 * @author Vida Wang (support@vida.wang)
 * @brief This file packed some functions to access flash.
 * User don't need to access these functions directly, using 
 * functions in mydb.h to instead.
 * 
 * 这个文件包装了一堆函数来使用flash。
 * 用户不需要直接访问这里的函数，使用mydb.h中的功能来替代。
 * 
 * @version 1.0.0
 * @date 2022-08-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>
#include <vector>
#include <stdint.h>
#include <mycrypto.h>

using namespace std;

typedef struct
{
    String name;
    int size;
    bool isFile;
} FileElement;

#define FORMAT_LITTLEFS_IF_FAILED true
typedef std::vector<FileElement> fileElementList;

class MyFS
{
public:
    static void myfsInit();
    static void listFile(String path, fileElementList *list, String prefix = "");
    static bool writeFile(String path, String data, bool base64Encode = true);
    static bool writeFile(const char *path, uint8_t *data, uint64_t length);
    static bool writeFile(const char *path, const char *data, bool base64Encode = true);
    static String readFile(String path, bool base64Decode = true);
    static void readFile(const char *p, std::function<void(uint8_t *output, uint64_t length)> callback);
    static uint8_t *readFile(const char *p, uint64_t *outLen);
    static bool appendFile(String path, String data);
    static bool deleteFile(String path);
    static bool renameFile(String path0, String path1);
    static bool fileExist(String path);
    static void formatSPIFFS();
    static size_t getFreeSpace();
    static size_t getUsedSpace();
};