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
    // plain text
    // 明文
    String plain = "12345678";

    Serial.printf("Plain text: %s\n", plain.c_str());
    Serial.printf("明文: %s\n", plain.c_str());

    // key, 32 bytes
    // 密匙, 32字节
    String key = "abcdefghijklmnopqrstuvwxyz012345";

    // iv, 16 bytes
    // 初始化向量, 16字节
    String iv = "0123456789abcdef";

    // encrypt
    // 加密
    String cipher = mycrypto::AES::aes256CBCEncrypt(
        key,
        iv,
        plain);

    // the cipher is hex format, lower case
    // 密文是16进制字符串，小写
    Serial.printf("Encrypted data: 0x%s\n", cipher.c_str());
    Serial.printf("加密后的数据: 0x%s\n", cipher.c_str());

    // decrypt
    // 解密
    String decrypted = mycrypto::AES::aes256CBCDecrypt(
        key,
        iv,
        cipher);

    Serial.printf("Decrypted data: %s\n", decrypted.c_str());
    Serial.printf("解密后的数据: %s\n", decrypted.c_str());

    // ==========
    // you could also use traditional method
    // 你也可以使用传统方法

    const char *plain2 = "abc";

    uint32_t encryptOutLen = 0;

    uint8_t *encryptedBuffer = mycrypto::AES::aes256CBCEncrypt(
        (uint8_t *)key.c_str(), // key 密匙
        (uint8_t *)iv.c_str(),  // iv 初始化向量
        (uint8_t *)plain2,      // plain 明文
        strlen(plain2),         // length of plain 明文长度
        &encryptOutLen          // length of output 输出长度
    );

    // output buffer will be set to nullptr
    // and output length will be set to 0
    // if error occured

    // 如果发生错误，buffer会返回空指针
    // 输出长度会返回0

    if (encryptedBuffer && encryptOutLen)
    {
        // do your thing with buffer and length
        // 你可以使用 buffer 和 长度做你想做的事

        // here BTW, for example, decrypt it
        // 这里顺便演示解密

        // decrypting part
        // 解密部分
        uint32_t decryptedOutLen = 0;

        uint8_t *decryptedBuffer = mycrypto::AES::aes256CBCDecrypt(
            (uint8_t *)key.c_str(), // key 密匙
            (uint8_t *)iv.c_str(),  // iv 初始化向量
            encryptedBuffer,        // cipher 密文
            encryptOutLen,          // length of cipher 密文的长度
            &decryptedOutLen        // length of output 输出的长度
        );

        if (decryptedBuffer && decryptedOutLen)
        {
            // do your thing with buffer and length
            // 你可以使用 buffer 和 长度做你想做的事

            delete decryptedBuffer;
            decryptedOutLen = 0;
        }
        else
        {
            Serial.println("Error occured when decrypting\n");
            Serial.println("解密时发生错误\n");
        }

        // decryption end
        // 解密部分结束

        // don't forget release memory
        // 别忘记释放内存
        delete encryptedBuffer;

        // reset outLen to 0 is more better
        // 最好连输出长度也重置
        encryptOutLen = 0;
    }
    else
    {
        Serial.println("Error occured when encrypting\n");
        Serial.println("加密时发生错误\n");
    }
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