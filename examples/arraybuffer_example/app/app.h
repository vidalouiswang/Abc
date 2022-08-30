#pragma once
#include <Arduino.h>
#include <config.h>

/**
 * @brief name of app
 *
 * app名称
 *
 */
#define APP_NAME "Class ArrayBuffer"

/**
 * @brief version of app
 *
 * app版本
 *
 */
#define APP_VERSION 0

/**
 * @brief default cpu frequency of current app
 *
 * 这个app默认的cpu频率
 *
 */
#define DEFAULT_CPU_FREQ 240

/**
 * @brief indicate app has setup function
 * or not
 *
 * 指示app是否有初始化函数
 *
 */
#define APP_HAS_SETUP

/**
 * @brief indicate app has main loop function
 *
 * 指示app是否有主循环函数
 *
 */
#define APP_HAS_LOOP

/**
 * @brief indicate app has custom extra callback
 * for local websocket server
 *
 * 指示app是否有自定义的用于本地websocket服务器的回调函数
 *
 *
 * @note the function declared and defined as C pattern
 * not included in class App
 *
 * 函数以C语言风格声明和定义，不包含在类App内
 *
 */
//#define APP_HAS_EXTRA_LOCAL_WEBSOCKET_CALLBACK

/**
 * @brief indicate app has custom extra callback
 * for remote websocket client
 *
 * 指示app是否有自定义的用于远程websocket客户端的回调函数
 *
 *
 * @note the function declared and defined as C pattern
 * not included in class App
 *
 * 函数以C语言风格声明和定义，不包含在类App内
 *
 */
//#define APP_HAS_EXTRA_REMOTE_WEBSOCKET_CALLBACK

/**
 * @brief default app class
 * you could modify class name, attention attached
 *
 * 默认的app类
 * 你可以修改这个类的名字，请阅读注意事项
 *
 * @attention the name of instance must be "app"
 * refer to the tail of the file "app.cpp"
 *
 * 类实例的名称必须为 "app"，查看 "app.app" 的尾部
 *
 * @note if you modified the name of he instance,
 * you should also modify the name in the file "main.cpp"
 * not recommended for beginner
 *
 * 如果你修改了类实例的名字，你也应该一同修改文件 "main.cpp"
 * 里的名字
 * 不推荐新手做此操作
 *
 */
class App
{
private:
public:
    /**
     * @brief default constructor, attention attached!
     * 默认构造函数，请阅读注意事项!
     *
     * @attention if you declared and define other constructors
     * you should NOT do any actions except set default value
     * for variables.
     * Because the instance of class App will be define in the tail
     * of the app.cpp, and that is NOT included in the system
     * GlobalManager::makeSureNewFirmwareValid() measurement scope,
     * if any action in constructor cause ESP32 panic reset,
     * you will lost connection of this ESP32, then you have to
     * use hardware serial to upload new firmware.
     *
     * 如果你定义了其他构造函数，请确保在构造函数内 [不要] 做 除了给变量赋值
     * 以外的其他操作，因为类App的实例在 app.cpp 文件的尾部被实例化，
     * 而这不在系统 GlobalManager::makeSureNewFirmwareValid() 的
     * 监测范围内，如果构造函数内有任何操作导致ESP32复位，你会丢失与这个
     * ESP32的连接，此时你只能再次用硬件串口重新上传固件。
     *
     */
    inline App(){};
    inline ~App(){};
    void setup();
    void loop();
};

extern App *app;