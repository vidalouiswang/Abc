#include "app.h"
#include "../src/globalmanager/globalmanager.h"

void App::setup()
{
    // a temporary array
    // 一个临时数组
    uint8_t buffer[128] = {0};

    // set some ramdom value
    // 设置点随机数
    for (uint32_t i = 0; i < 128; ++i)
    {
        buffer[i] = random(0, 0xff);
    }

    // container
    // 容器
    Elements container =
        {
            new Element(0x01),
            new Element(0x80),
            new Element(0xffff),
            new Element(0xffffffff),
            new Element("Hello world!"),
            new Element(buffer, 1024)};

    // method A
    // 方法A
    ArrayBuffer::createArrayBuffer(
        &container,
        [](uint8_t *outputBuffer, uint64_t length, bool *isBufferDeleted)
        {
            // this buffer could be send through socket of store to flash
            // 这个数组可以被通过网络发送或存储到flash

            // this buffer will be freed automatically
            // 这个数组会被自动删除

            // or you could delete it manually
            // 或者你也可以手动删除
            delete outputBuffer;
            (*isBufferDeleted) = true;
        });

    // method B
    // 方法B
    uint64_t outLen = 0;
    uint8_t *outputBuffer = ArrayBuffer::createArrayBuffer(&container, &outLen);

    if (outputBuffer && outLen)
    {
        // buffer OK

        ArrayBuffer::decodeArrayBuffer(
            [](Elements *outputVector)
            {
                if (outputVector)
                {
                    if (outputVector->size())
                    {
                        for (
                            Elements::iterator it = outputVector->begin();
                            it != outputVector->end();
                            ++it)
                        {
                            auto type = (*it)->getType(true);
                            if (type == NUMBER)
                            {
                                Serial.printf("number: %llu\n", (*it)->getNumber());
                            }
                            else if (type == STRING)
                            {
                                //Serial.printf("string: %s\n", (*it)->getString().c_str());
                                // or 或者
                                Serial.printf("string: %s\n", (*it)->c_str());
                            }
                            else if (type == U8A)
                            {
                                Serial.printf("uint8 array:\n 0x%s\n", (*it)->getHex());
                            }
                            else
                            {
                                Serial.println("error");
                            }
                        }
                    }
                }
            },
            outputBuffer,
            outLen);
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