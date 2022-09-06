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
    // SHA1
    String sha1Result = mycrypto::SHA::sha1("1234567890");

    // SHA256
    String sha256Result = mycrypto::SHA::sha256("1234567890");

    // print
    // 打印
    Serial.printf("SHA1 of \"1234567890\": %s\n", sha1Result.c_str());
    Serial.printf("SHA256 of \"1234567890\": %s\n", sha256Result.c_str());

    //and there are more way to get digest, refer to /lib/mycrypto/mycrypto.h
    //还有更多方法获取数字摘要，访问 /lib/mycrypto/mycrypto.h
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