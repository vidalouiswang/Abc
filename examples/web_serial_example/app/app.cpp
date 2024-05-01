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

    app->push = dbApp("push")->getUint8() ? true : false;

    // create a provider for toggle message push
    // 创建一个 provider 用于切换消息推送
    global->createProvider(
        [](ProviderArguments arguments) -> Element *
        {
            app->push = !app->push;

            // you could store it with database
            // 你可以顺便把设置保存一下

            *dbApp("push") = app->push ? 1 : 0;
            dbApp.flush();

            return new Element("OK");
        },
        "Toggle 开关", PROVIDER_USER);
}

void App::loop()
{
    if (this->push)
    {
        uint64_t t = millis();

        if (t - this->lastPushTime > 1000)
        {
            this->lastPushTime = t;
            global->sendMessageToClient(globalTime->getTime());
        }
    }
}

/**
 * @brief App instance defined here
 *
 * 类App实例在此实例化
 *
 */
App *app = new App();