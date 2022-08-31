#include "app.h"
#include "../src/globalmanager/globalmanager.h"

void App::setup()
{
    // set GPIO mode
    // 设置GPIO模式
    pinMode(this->pinLED, OUTPUT);
    digitalWrite(this->pinLED, LOW);

    // check delay timeout valid
    // 检查延时是否被误设置成其他值
    if (!this->delayTimeout || this->delayTimeout > 100000)
        this->delayTimeout = 1000U;
}

void App::loop()
{
    // method A
    // 方法A
    // digitalWrite(this->pinLED, HIGH);
    // delay(this->delayTimeout);
    // digitalWrite(this->pinLED, LOW);
    // delay(this->delayTimeout);

    // method B
    // 方法B
    uint64_t t = millis();
    if (t - this->lastActionTime > this->delayTimeout)
    {
        this->lastActionTime = t;
        if (this->isLightOn)
        {
            digitalWrite(this->pinLED, LOW);
        }
        else
        {
            digitalWrite(this->pinLED, HIGH);
        }
        this->isLightOn = !(this->isLightOn);
    }
}

/**
 * @brief App instance defined here
 *
 * 类App实例在此实例化
 *
 */
App *app = new App();