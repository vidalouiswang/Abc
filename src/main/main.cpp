// DO NOT change following contents in this area if you are a beginner
#include "../app/app.h"
#include "../src/globalmanager/globalmanager.h"

void setup()
{
#ifdef APP_HAS_EXTRA_LOCAL_WEBSOCKET_CALLBACK
  // extra local(AP) websocket callback
  // if command missed, it will be called
  global->setExtraLocalWebsocketCallback(extraLocalWebsocketCallback);
#endif
#ifdef APP_HAS_EXTRA_REMOTE_WEBSOCKET_CALLBACK
  // extra remote(WiFi) websocket callback
  // if command missed, it will be called
  global->setExtraRemoteWebsocketCallback(extraRemoteWebsocketCallback);
#endif

#ifdef APP_HAS_SETUP
  // register app setup
  global->registerAppSetup(
      []()
      {
        app->setup();
      });
#endif

#ifdef APP_HAS_LOOP
  // register app loop
  global->registerAppLoop(
      []()
      {
        app->loop();
      });
#endif

  // setup all things
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