/**
 * @file extra.h
 * @author Vida Wang (support@vida.wang)
 * @brief This header file included commands and some settings
 * Beginners should only modify those settings could be modified.
 *
 * 这个头文件包含了命令和各种设置
 * 新手应该只修改允许修改的设置
 *
 * @version 1.0.0
 * @date 2022-07-25
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

// version of system
// 主框架版本号
#define SYSTEM_VERSION 42

// debug system switch
// 主框架调试开关
//#define SYSTEM_DEBUG_ON

// a timeout for auto reconnect after wifi access point offline
// wifi掉线后自动重连的时间间隔
#define AUTO_CONNECT_WIFI_TIMEOUT 5000

// a timeout that reboot esp32 when ota failed for unknown reason
// 当ota由于不明原因升级失败时重启的超时
#define OTA_UPDATE_HARD_RESET_TIMEOUT 1000 * 60 * 5

// for authorize process, how long when a timestamp accepted that consider it is illegal
// 认证身份时收到的时间戳多久以前算非法时间戳
#define ILLEGAL_TIMESPAN_TIMEOUT 600 * 1000

/**
 * @brief
 * the number that how many used hashes will stored in current esp32
 * this value should set with ILLEGAL_TIMESPAN_TIMEOUT
 * large value will consume much heap memory
 * tiny value will consume less heap memory
 *
 * 设置存储多少个使用过的哈希值，这个值应该与 ILLEGAL_TIMESPAN_TIMEOUT 一同配合设置
 * 更大的值会使用更多的堆内存
 * 更小的值会使用更少的堆内存
 */
#define MAXIMUM_USED_HASH_CAPACITY 300

/**
 * @brief preset wifi ssid
 * 
 * 预设wifi名称
 *
 */
//#define PRESET_WIFI_SSID "WIFI_NAME"

/**
 * @brief preset wifi password
 * 
 * 预设wifi密码
 * 
 */
//#define PRESET_WIFI_PASSWORD "12345678"

/**
 * @brief you can preset user name for administrator here, attention attached
 * 你可以在这里预设管理员账号，请阅读注意事项
 *
 * @note format: 64bytes hex string, SHA256 digest of the user name, lower case
 * 格式: 64字节16进制字符串, 用户名的SHA256数字摘要, 小写
 *
 * @attention this action not recommended
 * 不推荐使用此操作
 *
 */
//#define PRESET_ADMIN_USERNAME ""

/**
 * @brief you can preset password for administrator here, attention attached
 * 你可以在这里预设管理员密码，请阅读注意事项
 *
 * @note format: 64bytes hex string, SHA256 digest of the password, lower case
 * 格式: 64字节16进制字符串, 密码的SHA256数字摘要, 小写
 *
 * @attention this action not recommended
 * 不推荐使用此操作
 *
 */
//#define PRESET_ADMIN_PASSWORD ""

/**
 * @brief if you want to use prset information
 * for websocket connection, you need to enable this
 * definition, attention attached
 *
 * 如果你想使用预设的websocket信息，你需要打开这个define
 * 请阅读注意事项
 *
 * @attention if websocket connection information had been
 * detected in database, the preset information will NOT be
 * use as valid information.
 *
 * 如果在数据库中检索到了websocket连接信息，则预设信息不会被使用。
 */
//#define ENABLE_USE_PRESET_WEBSOCKET_INFORMATION

/**
 * @brief preset domain for remote websocket
 * 预设的websocket域名
 *
 */
//#define PRESET_WEBSOCKET_DOMAIN ""

/**
 * @brief preset port for remote websocket
 * 预设的websocket端口
 *
 */
//#define PRESET_WEBSOCKET_PORT 0

/**
 * @brief preset path for remote websocket
 * 预设的websocket路径
 *
 */
//#define PRESET_WEBSOCKET_PATH "/"

/**
 * @brief to enable SHA1 digest for authorization, attention attached
 * 允许使用SHA1进行权限认证，请阅读注意事项
 *
 * @note because of the iOS and MacOS shortcut don't
 * support SHA256 calculation, so you can use SHA1 for
 * authorization
 * 
 * Now Apple shortcut already support SHA256
 *
 * 因为iOS和MacOS的捷径无法进行SHA256运算，所以你可以打开此选项
 * 以允许使用SHA1进行认证
 * 
 * 现在苹果捷径已支持SHA256
 *
 * @attention SHA1 is not safe for user authorization
 * make sure you really want to open this settings
 *
 * SHA1用于用户认证不安全，请确定是否真的想使用此选项
 *
 */
//#define ENABLE_SHA1_AUTHORIZATION

#ifdef ENABLE_SHA1_AUTHORIZATION
#define SHA_LENGTH 20
#else
#define SHA_LENGTH 32
#endif

/**
 * @brief SSID prefix for AP mode
 * AP模式SSID前缀
 *
 */
#define AP_PREFIX "ABC"

/**
 * @brief preset password for AP mode, attention attached
 * AP模式的预设密码，请阅读注意事项
 *
 * @attention AT LEAST 8 bytes
 * [最少] 8个字符
 */
#define PRESET_AP_PASSWORD "12345678"

/**
 * delay start ap
 * access point radio circuit will consume large power current in a very short time
 * that may cuase esp32 low voltage reset
 * you have to ensure your hardware has enough power supply for esp32 for STA+AP on boot
 * at least 44uf capacitor required if your hardware uses ldo that provide 3.3v(typ), 600ma for esp32
 * otherwise one 22uf capacitor is enough(two capacitors recommended)
 * so this action will delay DELAY_START_AP_TIMEOUT milliseconds
 *
 * 延迟启动AP
 * AP电路会在短时间内消耗大量的电流
 * 这可能会触发esp32低电压复位
 * 你需要确保你的硬件在启动时使用STA+AP模式时有足够的功率
 * 如果使用3.3v, 600ma的ldo供电，则至少需要44uf电容
 * 电源供应充足时，1个22uf电容就够用了(推荐两个)
 * 所以开启AP的操作会延迟 DELAY_START_AP_TIMEOUT 毫秒
 */
#define ENABLE_DEALY_START_AP

// timeout for delay start ap broadcast
// 延迟启动ap的超时时间
#define DELAY_START_AP_TIMEOUT 8000

// proactive detect remote server online or not
// 是否主动探测远程服务器是否在线
#define PROACTIVE_DETECT_REMOTE_SERVER

/**
 * @brief if PROACTIVE_DETECT_REMOTE_SERVER is defined
 * ap will be open or closed automatically
 *
 * 如果定义了 PROACTIVE_DETECT_REMOTE_SERVER
 * ap会自动开启和关闭
 *
 */
#ifdef PROACTIVE_DETECT_REMOTE_SERVER
#define AUTOMATIC_CLOSE_AP_IF_REMOTE_WEBSOCKET_CONNECTED
#ifdef AUTOMATIC_CLOSE_AP_IF_REMOTE_WEBSOCKET_CONNECTED
#define AUTOMATIC_START_AP_IF_REMOTE_WEBSOCKET_DISCONNECTED
#endif
#endif

// timeout and count number for auto close ap if esp32 got stable connection to remote server
// esp32稳定连接到远程服务器后自动关闭ap的超时时间和计数
#ifdef AUTOMATIC_START_AP_IF_REMOTE_WEBSOCKET_DISCONNECTED
#define AUTOMATIC_START_AP_IF_REMOTE_WEBSOCKET_DISCONNECTED_TIMES 5
#define AUTOMATIC_CLOSE_AP_IF_REMOTE_WEBSOCKET_CONNECTED_TIMEOUT 1000 * 60
#endif

// add built-in provider for basic functions if defined
// 如果定义了则会添加基础功能的功能提供器
#define BUILT_IN_FUNCTION_PROVIDER

// reboot automatically if remote server offline for a long time
// 如果远程服务器很长时间不在线自动重启
#define REBOOT_IF_PROACTIVE_DETECT_REMOTE_SERVER_OFFLINE

// a count number if remote server offline for a long time
// 如果远程服务器很长时间不在线自动重启的计数
#define REBOOT_IF_PROACTIVE_DETECT_REMOTE_SERVER_OFFLINE_TIMES 100

// how long to send a packet to detect server is online
// 多长时间发送一个数据包检测服务器是否在线
#define DETECT_SERVER_ONLINE_TIMEOUT 10000

// how long that server response esp32 detectation data packet
// 服务器需要在多长时间内响应esp32发出的检测服务器是否在线的数据包
#define SERVER_RESPONSE_TIMEOUT 3000

/**
 * @brief
 * espressif esp-idf has a feature could rollback firmware that through OTA update
 * if new firmware is invalid, like reset in "pending verify" state, but according to
 * code of that arduino framework, new firmware will be set to valid for always
 * to ensure code could be copy everywhere on different develop enviorment
 * I didn't modify basic framework, because of those codes will be download
 * by pio, so I added this function in another way, it able to rollback
 * firmware automatically if new firmware has fatal error on boot, it could ensure
 * your esp32 always controllable, once firmware has error and reset times greater
 * than this value, it will rollback, that ensure you can do another ota update
 *
 * 乐鑫的esp-idf框架有一个功能可以自动回滚在"pending verify"状态发生异常复位的固件
 * 但是这个功能在arduino框架中没有开启，sdk中的代码会直接把OTA升级的新固件状态设置为可用
 * 这样自动回滚功能就无法使用。为了保证代码在不同的开发机器上的可移植性，我没有修改官方框架的代码
 * 因为那些代码是pio自动下载的，所以我使用了另一种方法来实现当固件在被监测状态发生复位时可以
 * 自动回滚的功能，这样esp32就不会失联了，一旦新的固件在监测期间复位次数超过设定的阈值，框架自动
 * 回滚固件，这样可以从容不迫的找出代码错误再更新一个固件上去
 *
 */
#define ERROR_BOOT_COUNT_VALVE 3

// how long that consider current firmware is valid, start from connected to remote server
// 多长时间之后认为当前固件是有效的，从连接到服务器开始计算
#define CONFIRM_NEW_FIRMWARE_VALID_TIMEOUT 1000 * 600 // ten minutes to confirm ota firmware is valid

// debug header
// 调试头
#define SYSTEM_DEBUG_HEADER "ABC"

// the main loop will run in a single RTOS task if defined
// 如果定义了则主循环会运行在单独的RTOS任务中
//#define SINGLE_TASK_RUN_MAINLOOP
#ifdef SINGLE_TASK_RUN_MAINLOOP
#define MAIN_LOOP_TASK_PRIORITY 2
#define MAIN_LOOP_TASK_CORE 1
#define MAIN_LOOP_STACK_SIZE 16 * 1024
#endif

// one OTA data block timeout if server didn't response our request
// 如果服务器没有响应esp32发出的请求一个OTA数据块的请求的超时时间
#define OTA_BLOCK_REQUESTED_TIMEOUT 10000

// you could choose which version you want to build here, English or Chinese
// 你可以在这里选择你想构建的固件版本，中文或英文
//#define ENGLISH_VERSION
#define CHINESE_VERSION

#ifdef CHINESE_VERSION
#ifdef ENGLISH_VERSION
#undef ENGLISH_VERSION
#endif
#endif

#ifndef ENGLISH_VERSION
#ifndef CHINESE_VERSION
#define CHINESE_VERSION
#endif
#endif

// length of single ota data block vector
// 单个ota数据块包元素的个数
#define OTA_DATA_BLOCK_VECTOR_LENGTH 4

// length of find device vector
// 查找设备的数据包中元素的个数
#define FIND_DEVICE_VECTOR_LENGTH 5

// commands
// 命令

// basic command offset in a data packet
// 基础命令在数据包中的位置
#define OFFSET_COMMAND 0

// offset related to ota function
// ota功能的元素数据位置
// hash offset
// 哈希位置
#define OFFSET_OTA_HASH 1

// data offset
// 数据块位置
#define OFFSET_OTA_DATA 2

// index offset, this index indicate current ota block index
// 索引位置，这个索引是当前ota数据块的编号
#define OFFSET_OTA_BLOCK_INDEX 3

// the following offsets used in start ota function part
// 下面的位置偏移是在开始ota功能时使用的

// admin id that started ota
// 执行ota升级的管理员id
#define OFFSET_START_OTA_ADMIN 1

// length of single ota data block
// 单个ota数据块的长度
#define OFFSET_START_OTA_BLOCK_SIZE 2

// length of whole firmware
// 固件整体大小
#define OFFSET_START_OTA_FIRMWARE_LENGTH 3

// timestamp for authorize
// 用于认证的时间戳
#define OFFSET_START_OTA_TIMESTAMP 4

// hash for authorize
// 用于认证的哈希
#define OFFSET_START_OTA_HASH 5

// timeout of the whole process of ota
// if esp32 didn't finish ota update process in time
// it will reboot
// 整个ota升级过程的超时时间
// 如果没有在规定时间内完成升级则重启
#define OTA_WHOLE_PROCESS_TIMEOUT 600 * 1000

// for reponse to server that message has been confirmed
// 用于回应服务器消息已经收到
#define CMD_CONFIRM 0xA000000000000000ULL

// for keep alive
// 用于保活
#define CMD_HELLO 0x0C
#define CMD_WORLD 0xC0

/**
 * @brief esp32 will send this command to server after
 * wifi and websocket connected, register to server self
 * id, then server will send back real time(unix epoch)
 * to esp32
 *
 * 当esp32上电后，wifi和websocket连接成功后，esp32会发送此命令
 * 到服务器以注册自己的id，服务器会发回unix时间戳到esp32用以同步时间
 *
 */
#define CMD_REGISTER_OR_ROLE_AUTHORIZE 0x80

// administrator pushed a request to esp32 require
// esp32 start ota process
// 管理员发送了ota升级请求到esp32要求esp32立即开始
// ota升级过程
#define CMD_OTA_WEBSOCKET 0xAB

// server send ota data segment to esp32
// 服务器发给esp32一个ota数据块
#define CMD_OTA_BLOCK 0xAC

// esp32 send this command to show information
// esp32发送此命令用于显示信息
#define CMD_LOG 0xFB

// user(admin or normal user) sent a request
// require basic information of current device
// 用户(管理员或普通用户)发送了请求
// 要求esp32发送自己的基本信息数据
#define CMD_FIND_DEVICE 0xAF

// esp32 send this command to server with basic information
// esp32 发送此命令和基本信息到服务器用以回应查找设备命令
#define CMD_FIND_DEVICE_RESPONSE 0xFA

// user require esp32 to execute a command
// esp32 will find related id of provider and execute it
// 用户要求esp32执行某个命令
// esp32会根据id号查找对应的功能提供器然后执行对应功能
#define CMD_EXECUTE_COMMAND 0xBB

// the following commands is for websocket server(ap mode)
// 下面的命令是给websocket服务器使用的(ap模式)

// user set basic information for current device
// like ssid and password of wifi, admin user name
// and password...
// 用户设置给当前设备设置基础信息，比如wifi名称和密码，管理员账号密码...
#define CMD_AP_SET_BASIC_INFORMATION 0x00

// user require current device reboot in 3 seconds
// 用户要求当前设备在3秒后重启
#define CMD_AP_DELAY_REBOOT 0x01

// user require current device reboot immediately
// 用户要求当前设备立即重启
#define CMD_AP_REBOOT_IMMEDIATELY 0x02

// user require current device rollback firmware
// 用户要求当前设备回滚固件
#define CMD_AP_ROLLBACK 0x88

// user require current device into deep sleeep
// 用户要求当前设备进入深度睡眠
#define CMD_AP_DEEPSLEEP 0x89

// the following 3 commands are not use currently
// 下面三个命令现在没有使用
#define CMD_AP_CONNECT_WIFI 0x90
#define CMD_AP_WIFI_UNAVAILABLE 0x10
#define CMD_AP_WIFI_CONNECTED 0X11