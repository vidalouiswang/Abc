#include "app.h"
#include "../src/globalmanager/globalmanager.h"

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
}

/**
 * @brief App instance defined here
 *
 * 类App实例在此实例化
 *
 */
App *app = new App();