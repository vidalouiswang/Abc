#include "app.h"
#include "../src/src/globalmanager/globalmanager.h"

void App::setup()
{
    MyDB dbExample("example");
    dbExample.begin();

    if (!dbExample.loaded)
    {
        Serial.println("Database load failed 数据库加载失败");
        return;
    }

    auto value = dbExample("key");
    if (!value->available())
    {
        Serial.println("key does not exists or value unavailable\n键不存在或值不可用");
    }

    // set value
    // 设置值
    (*value) = "value";

    // or set it directly
    // 或者直接设置

    *dbExample("number_A") = 0;

    // force type
    // 强制类型
    *dbExample("number_B") = (uint64_t)0xffU;

    *dbExample("number_C") = (uint64_t)millis();

    (*dbExample("number_A"))++;

    // clear a key/value
    // 清除键/值
    *dbExample("number_C") = "";

    // don't forget write it to flash
    // 别忘了写入flash
    if (!dbExample.flush())
    {
        Serial.println("database flush failed\n数据库写入失败");
    }
}

void App::loop()
{
}

/**
 * @brief App instance defined here
 *
 * 类App实例在此实例化
 *
 */
App *app = new App();