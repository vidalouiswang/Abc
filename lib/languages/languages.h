/**
 * @file languages.h
 * @author Vida Wang (support@vida.wang)
 * @brief This file include languages informations
 * 这个文件包含了语言信息
 * @version 1.0.0
 * @date 2022-08-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef LANGUAGES_H_
#define LANGUAGES_H_
#include "config.h"

#ifdef CHINESE_VERSION
// PROVIDER_INFO

#define PI_UNKNOWN "未知"
#define PI_BOOT "开机"
#define PI_ESP_RST_INT_WDT "中断看门狗"
#define PI_ESP_RST_TASK_WDT "任务看门狗"
#define PI_ESP_RST_WDT "其他看门狗"
#define PI_ESP_RST_PANIC "软件异常"
#define PI_ESP_RST_BROWNOUT "掉电保护器"
#define PI_ESP_RST_DEEPSLEEP "深度睡眠"
#define PI_ESP_RST_SW "软件复位"
#define PI_GET_RESET_AND_ONLINE_TIME "获取复位信息和启动时间"

#define PI_INVALID_LENGTH_OF_ARGUMENTS "参数个数不正确"
#define PI_INVALID_LENGTH_OF_ARGUMENT "参数长度不正确"
#define PI_INVALID_ARGUMENT "参数不正确"
#define PI_INVALID_TYPE_OF_ARGUMENT "参数类型不正确"
#define PI_SUCCESS "成功"
#define PI_DELETED "已删除"

#define PI_TOKEN_SET "token已设置"
#define PI_MODIFY_TOKEN "修改token"

#define PI_USER_DELETED "用户删除成功"
#define PI_ADD_MODIFY_DELETE_USER "(添加/修改/删除)用户"

#define PI_WEB_SERIAL "发送数据到串口"

#define PI_DATABASE_UPDATED "数据库已更新"
#define PI_QUERY_UPDATE_MAIN_DATABASE_OR_LIST_ALL_KEYS_IN_MAIN_DATABASE \
    "((查询/更新)主数据库)/(列出所有主数据库中的所有键)"

#define PI_REMOVE_VALUE_IN_MAIN_DATABASE "删除主数据库中的值"

#define PI_WIFI_INFORMATION_REMOVED "WiFi信息已清除"
#define PI_REMOVE_WIFI_INFORMATION "[清除WiFi信息]"

#define PI_CAN_NOT_ROLLBACK "没有可供回滚的固件"
#define PI_WILL_ROLLBACK_IN_TIMEOUT_SECONDS "会在3秒后回滚固件"
#define PI_ROLLBACK "[回滚固件]"

#define PI_FIRMWARE_PENDING_VERIFY "正在等待验证固件有效性"
#define PI_FIRMWARE_VALID "固件有效"
#define PI_NO_OTA_INFORMATION "没有在数据库中找到OTA升级信息"
#define PI_FIRMWARE_STATUS "固件状态"

#define PI_LAST_OTA_TIME "上次OTA更新时间"

#define PI_MAC_ADDRESS "物理地址(MAC)"

#define PI_WILL_INTO_DEEP_SLEEP_FOR_TIMEOUT_SECONDS_IN_SECONDS \
"将会睡眠%u秒, %u秒后执行"
#define PI_DEEP_SLEEP "[睡眠]"

#define PI_ALL_INFORMATION_REMOVED_REBOOT_IN_3_SECONDS \
    "所有信息已被清除, 3秒后重启"

#define PI_REMOVE_ALL_INFORMATION "[清除所有信息]"


#define PI_NEW_SSID_OR_PASSWORD_HAS_BEEN_SET "新的%s: \"%s\"已经设置"
#define PI_INVALID_SSID_PROVIDED "提供的SSID不正确"
#define PI_MODIFY_SSID "[修改WiFi SSID]"

#define PI_INVALID_WIFI_PASSWORD "提供了格式不正确的密码"
#define PI_MODIFY_WIFI_PASSWORD "[修改WiFi密码]"

#define PI_WILL_REBOOT_IN_TIMEOUT_SECONDS "将会在[%u]秒后重启"
#define PI_REBOOT "[重启]"

#define PI_HARDWARE_TIMESTAMP "硬件时间"

#define PI_PREFIX_BYTE "字节"
#define PI_FREE_HEAP "剩余堆内存"

#define PI_FREE_SPACE "剩余存储空间"

#define PI_FIRMWARE_ERROR_TIME "固件异常: %llu"


#else

#define PI_UNKNOWN "unknown"
#define PI_BOOT "power on"
#define PI_ESP_RST_INT_WDT "interrupt watchdog"
#define PI_ESP_RST_TASK_WDT "task watchdog"
#define PI_ESP_RST_WDT "other watchdogs"
#define PI_ESP_RST_PANIC "software error"
#define PI_ESP_RST_BROWNOUT "low voltage"
#define PI_ESP_RST_DEEPSLEEP "deep sleep"
#define PI_ESP_RST_SW "software reset"
#define PI_GET_RESET_AND_ONLINE_TIME "Get last reset reason"

#define PI_INVALID_LENGTH_OF_ARGUMENTS "Invalid length of arguments"
#define PI_INVALID_LENGTH_OF_ARGUMENT "Invalid length of argument"
#define PI_INVALID_ARGUMENT "Invalid argument"
#define PI_INVALID_TYPE_OF_ARGUMENT "Invalid type of arguments"
#define PI_SUCCESS "Success"
#define PI_DELETED "Removed"

#define PI_TOKEN_SET "Token has been set"
#define PI_MODIFY_TOKEN "Modify token"

#define PI_USER_DELETED "User removed"
#define PI_ADD_MODIFY_DELETE_USER "(Add/Update/Remove)User"

#define PI_WEB_SERIAL "Send data to serial"

#define PI_DATABASE_UPDATED "Database updated"
#define PI_QUERY_UPDATE_MAIN_DATABASE_OR_LIST_ALL_KEYS_IN_MAIN_DATABASE \
    "((Query/Update)value in main database)/(List all keys in main database)"

#define PI_REMOVE_VALUE_IN_MAIN_DATABASE "Remove value in main database"

#define PI_WIFI_INFORMATION_REMOVED "WiFi information has been removed"
#define PI_REMOVE_WIFI_INFORMATION "[RESET WiFi information]"

#define PI_CAN_NOT_ROLLBACK "There isn't firmware could rollback"
#define PI_WILL_ROLLBACK_IN_TIMEOUT_SECONDS "Will rollback firmware in 3 seconds"
#define PI_ROLLBACK "[ROLLBACK FIRMWARE]"

#define PI_FIRMWARE_PENDING_VERIFY "Pending verify"
#define PI_FIRMWARE_VALID "Firmware OK"
#define PI_NO_OTA_INFORMATION "Couldn't detect pendingOTA in database"
#define PI_FIRMWARE_STATUS "Firmware status"

#define PI_LAST_OTA_TIME "Last OTA update time"

#define PI_MAC_ADDRESS "MAC Address"

#define PI_WILL_INTO_DEEP_SLEEP_FOR_TIMEOUT_SECONDS_IN_SECONDS \
"Will into deep sleep for [%u] seconds in %u seconds"
#define PI_DEEP_SLEEP "[SLEEP]"

#define PI_ALL_INFORMATION_REMOVED_REBOOT_IN_3_SECONDS \
    "All information has been removed, reboot in 3 seconds"

#define PI_REMOVE_ALL_INFORMATION "[REMOVE ALL INFORMATION]"


#define PI_NEW_SSID_OR_PASSWORD_HAS_BEEN_SET "New%s: \"%s\" set"
#define PI_INVALID_SSID_PROVIDED "Invalid SSID provided"
#define PI_MODIFY_SSID "[UPDATE WiFi SSID]"

#define PI_INVALID_WIFI_PASSWORD "Invalid password provided"
#define PI_MODIFY_WIFI_PASSWORD "[UPDATE WiFi password]"

#define PI_WILL_REBOOT_IN_TIMEOUT_SECONDS "Will reboot in %u second(s)"
#define PI_REBOOT "[REBOOT]"

#define PI_HARDWARE_TIMESTAMP "Hardware timestamp"

#define PI_PREFIX_BYTE "bytes"
#define PI_FREE_HEAP "Free heap"

#define PI_FREE_SPACE "Free space"

#define PI_FIRMWARE_ERROR_TIME "Firmware error: %llu"


#endif

#endif