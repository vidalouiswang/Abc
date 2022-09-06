/**
 * @file app.cpp
 * @brief
 *
 * Code this file just like Arduino way
 *
 * 就像使用Arduino框架一样在这个文件中写代码就可以了
 *
 */

#include "app.h"

void App::setup()
{
    /**
     * ===English:
     * put your code in this function to run it once
     * 
     * ===中文：
     * 把你的代码放在这个函数里，代码会被运行一次
     */

    /**
     * ===English:
     * if you just did an OTA update
     * you could do anything here, don't worry any code bugs
     * if any error happened, and reboot times reach maximum valve
     * firmware will rollback to former version automatically
     * 
     * you could modify the valve yourself
     * in file /lib/config/config.h
     * 
     * ! Attention: ONLY when firmware updated by built-in OTA function
     * has this feature, not included update through serial lines
     * 
     * ===中文：
     * 如果你刚才使用了OTA升级了这个固件
     * 你可以在这里写任何代码
     * 无须担心任何错误引起无限复位
     * 当复位次数超过设定阈值时
     * 固件会自动回滚上一个版本
     * 
     * 阈值可以根据自己的需求调整
     * 在 /lib/config/config.h
     * 
     * ! 注意: [ 仅 ] 当固件通过内置的OTA升级功能升级后才有此功能
     * 直接从串口升级的固件没有此功能
     */
}

void App::loop()
{
    // put your code in this function
    // it will run repeatedly, attention attached

    // 把你的代码放在这个函数里，代码会重复的运行，请阅读注意事项
}

/**
 * @brief App instance defined here
 * 
 * 类App实例在此实例化
 * 
 */
App *app = new App();