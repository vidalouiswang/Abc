// DO NOT change following contents in this area if you are a beginner
// 如果你是新手，[不要] 修改下面的任何内容
#include "../app/app.h"
#include "../src/globalmanager/globalmanager.h"

void setup()
{
#ifdef APP_HAS_EXTRA_LOCAL_WEBSOCKET_CALLBACK
  // extra local(AP) websocket callback
  // if built-in command missed, it will be called
  // 额外的 本地(AP) websocket 回调函数
  // 如果内置命令没有命中，这个回调函数会被执行
  global->setExtraLocalWebsocketCallback(extraLocalWebsocketCallback);
#endif
#ifdef APP_HAS_EXTRA_REMOTE_WEBSOCKET_CALLBACK
  // extra remote(WiFi) websocket callback
  // if built-in command missed, it will be called
  // 额外的 远程(WiFi) websocket 回调函数
  // 如果内建命令没有命中，这个回调函数会被执行
  global->setExtraRemoteWebsocketCallback(extraRemoteWebsocketCallback);
#endif

#ifdef APP_HAS_SETUP
  // register app setup
  // 注册 app 的初始化函数
  global->registerAppSetup(
      []()
      {
        app->setup();
      });
#endif

#ifdef APP_HAS_LOOP
  // register app loop
  // 注册 app 的循环函数
  global->registerAppLoop(
      []()
      {
        app->loop();
      });
#endif

  // setup all things
  // 设置所有功能
  global->beginAll();
  Serial.println("OK");
}

void loop()
{
#ifndef SINGLE_TASK_RUN_MAINLOOP
  global->loop();
  yield();
#else
  yield();
#endif
}