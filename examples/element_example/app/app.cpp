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
    Element x;
    Element y = 100;
    uint8_t buf[] = {97, 98, 99};
    String hello = "Hello world!";

    // set value
    // 设置值

    // set number
    // 设置数字
    x = 10;

    x.setNumber(100);

    // set number with given type
    // 用指定的类型设置数字
    x = (uint16_t)65535;

    // set string
    // 设置字符串
    x = "Hello world!";
    x = hello;
    x = &hello;
    x.setString("Hello world");
    x.setString(hello);
    x.setString(&hello);

    // set buffer
    // 设置二进制数组
    x(buf, 3);
    x.setUint8Array(buf, 3);

    // set with another element
    // 用另一个元素来设置值
    x = y;
    x = &y;
    x.copyFrom(y);
    x.copyFrom(&y);

    // operators
    // 操作符

    // math operators
    // 数学符号

    // reset value
    // 重置值
    x = 10000;

    x++;
    x--;

    ++x;
    --x;

    uint64_t a = 0;

    a = x + y;
    a = x - y;
    a = x * y;
    a = x / y;
    a = x % y;

    a = x + 100;
    a = x - 100;
    a = x * 100;
    a = x / 100;
    a = x % 100;

    x = "Hello ";
    String b = x + String("world!");
    String c = "o";
    x += "world!";

    // will remove all char 'o'
    // 会去除掉所有的'o'
    String c = x - c;

    x -= c;

    // reset value
    // 重置值
    x = 10000;

    x += 100;
    x -= 100;
    x *= 100;
    x /= 100;
    x %= 100;

    // reset value
    // 重置值
    x = 10000;

    // bit operators
    // 位符号
    a = ~x;

    a = x << 2;
    a = x >> 2;

    x <<= 2;
    x >>= 2;

    a = x & 0xffU;
    a = x | 0xffU;

    x &= 0xffU;
    x |= 0xffU;

    a = x ^ 0xffU;
    x ^= 0xffU;

    // reset value
    // 重置值
    x = 10000;

    // logical
    // 逻辑值
    Element z = 2;
    bool d = x && 100;
    int e = 100;

    d = x || 100;
    d = !x;

    d = x > z;
    d = x < z;

    d = x >= z;
    d = x <= z;

    d = x == z;

    d = x > e;
    d = x < e;

    d = x <= e;
    d = x >= e;

    x = "abc";
    z = "cde";

    d = x < z;
    d = x > z;
    //...

    // reset value
    // 重置值
    x(buf, 3);
    z = 2;

    // index
    // 索引
    uint8_t f = x[2];
    f = x[z];

    x = "abc";
    char g = x[2];

    // others functions
    // 其他功能
    x = "Hello world!";
    const char *h = x.c_str();
    f = x.indexOf("w");
    f = x.lastIndexOf("o");

    ElementType type = x.getType();
    type = x.getType(true);

    // reset value
    // 重置值
    x = 10000;

    uint64_t i = x.getUint8();
    i = x.getUint16();
    i = x.getUint32();
    i = x.getUint64();
    i = x.getNumber();

    x = "abc";
    String j = x.getString();

    // reset value
    // 重置值
    x(buf, 3);
    String k = x.getHex();

    x = "0f3ab86d19f98891";
    x.convertHexStringIntoUint8Array();

    uint32_t m = 0;
    uint8_t *l = x.getUint8Array(&m);
    m = x.getRawBufferLength();

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