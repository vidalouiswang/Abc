#include <Arduino.h>
#include <unity.h>
#include <arraybuffer.hpp>
#include <vector>
#include <mycrypto.h>
#include <mydb.h>

void test_sha1()
{
    String a = "hello world!";
    String b = "Hello World!";
    String c = "Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!";
    TEST_ASSERT_EQUAL_STRING("430ce34d020724ed75a196dfc2ad67c77772d169", mycrypto::SHA::sha1(a).c_str());
    TEST_ASSERT_EQUAL_STRING("2ef7bde608ce5404e97d5f042f95f89f1c232871", mycrypto::SHA::sha1(b).c_str());
    TEST_ASSERT_EQUAL_STRING("89abb191ead1727513f76913920307fe3f994483", mycrypto::SHA::sha1(c).c_str());
}

void test_sha256()
{
    String a = "hello world!";
    String result = "7509e5bda0c762d2bac7f90d758b5b2263fa01ccbc542ab5e3df163be08e6ca9";

    TEST_ASSERT_EQUAL_STRING(result.c_str(), mycrypto::SHA::sha256(a).c_str());
}

void test_base64_encode()
{
    String a = "hello world!";
    String result = "aGVsbG8gd29ybGQh";

    TEST_ASSERT_EQUAL_STRING(result.c_str(), mycrypto::Base64::base64Encode(a).c_str());
}

void test_base64_decode()
{
    String a = "aGVsbG8gd29ybGQh";
    String result = "hello world!";

    TEST_ASSERT_EQUAL_STRING(result.c_str(), mycrypto::Base64::base64Decode(a).c_str());
}

void test_aes_encode()
{
    String key = "0123456789abcdef0123456789abcdef";
    String iv = "0123456789abcdef";

    String plain = "hello world!";
    Element plain2 = "Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!";
    String result = "700d435bf4c1ff6fbce7f8af99e8c1a9";
    String result2 = "oNPIp1ZoEgpzSqInur+jfc3SLjMclLNvQ8URWP2hmqueeIYmtHfTbnNaDJ3hNpBbYdNrpx5Hh0o6YHbfwd4WPE/6kYwD0GlK5IxSjCPu9OVIbEPcHHafI+Ks5QTZ+lSMbzc+20xNTAgoO65TCkxDzpdaAM6lJSyNK+xmgvAJbe8QI4RGIjFz633PGhTCMONFJADQTSXFGMUVn5m165VCWvAK6lqNLOUPKiaQfWgjEetgR/E0wZqUpdvxAf0Z76nPLdXEgEZAj1Pu5c0NFBtZTmKw8ogq8d+7wupnYpY/jgcj1avbYwAWbOUPMdapLydfQhhFXemj8UHWqvCrdFTZ3VVOPtLwPyV3BHeopf0VqbHriPhRwe6KGJz8IGbQNemer80F+NIwe1oLKf4y7vm5QOTo6eaPguDxt/jzuGzkIg8=";
    TEST_ASSERT_EQUAL_STRING(result.c_str(), mycrypto::AES::aes256CBCEncrypt(key, iv, plain).c_str());

    plain2.AES256_CBC(key.c_str(), iv.c_str());
    plain2.toBase64(true);

    TEST_ASSERT_EQUAL_STRING(plain2.c_str(), result2.c_str());
}

void test_aes_decode()
{
    String key = "0123456789abcdef0123456789abcdef";
    String iv = "0123456789abcdef";

    String cipher = "700d435bf4c1ff6fbce7f8af99e8c1a9";
    String result = "hello world!";
    TEST_ASSERT_EQUAL_STRING(result.c_str(), mycrypto::AES::aes256CBCDecrypt(key, iv, cipher).c_str());
}

void test_element_new_operators()
{
    char buf[128] = {0};

    Element a = 10;

    Element b = -2;

    uint8_t u8 = 1;
    int8_t i8 = -1;
    uint16_t u16 = 2;
    int16_t i16 = -2;
    uint32_t u32 = 4;
    int32_t i32 = -4;
    uint64_t u64 = 8;
    int64_t i64 = -8;
    float f = 1.23f;
    double d = 3.1415926f;



    // ==
    TEST_ASSERT_TRUE(a == 10);
    TEST_ASSERT_TRUE(b == -2);

    a = "Hello World!";
    TEST_ASSERT_TRUE(a == "Hello World!");

    // =
    a = 5;
    TEST_ASSERT_TRUE(a == 5);
    a = -5;
    TEST_ASSERT_TRUE(a == -5);
    a = -2147483666ll;
    TEST_ASSERT_TRUE(a == -2147483666ll);
    a = 0;
    TEST_ASSERT_TRUE(a == 0);
    a = f;
    TEST_ASSERT_TRUE(a == f);
    a = d;
    TEST_ASSERT_TRUE(a == d);

    a = "Hello World!";
    TEST_ASSERT_EQUAL_STRING("Hello World!", a.c_str());

    a = 5;
    b = 5;
    bzero(buf, sizeof(buf));
    sprintf(buf, "a type: %d, b type: %d, ai32: %d, bi32: %d, aram: 0x%x, bram:0x%x",
            a.getType(),
            b.getType(),
            a.getInt32(),
            b.getInt32(),
            a.getRAM(),
            b.getRAM());
    TEST_ASSERT_TRUE_MESSAGE(a.equalsTo(&b), buf);

    // !=
    TEST_ASSERT_TRUE(a != 6);
    TEST_ASSERT_TRUE(a != 2147483666llu);

    a = "Abc";
    TEST_ASSERT_TRUE(a != "def");

    // <
    a = 5;
    b = 8;
    TEST_ASSERT_TRUE(b == 8);
    TEST_ASSERT_TRUE(a < 6);
    TEST_ASSERT_TRUE(a < b);
    a = -8;
    TEST_ASSERT_TRUE(a < -2);

    a = "a";
    TEST_ASSERT_TRUE(a < "b");

    // >
    a = 3;
    b = 1;
    TEST_ASSERT_TRUE(b == 1);
    TEST_ASSERT_TRUE(a > 2);
    TEST_ASSERT_TRUE(a > b);
    a = -8;
    TEST_ASSERT_TRUE(a > -9);

    a = "c";
    TEST_ASSERT_TRUE(a > "a");

    // <=
    a = 5;
    b = 8;
    TEST_ASSERT_TRUE(b == 8);
    TEST_ASSERT_TRUE(a <= 6);
    TEST_ASSERT_TRUE(a <= 5);
    TEST_ASSERT_TRUE(a <= b);
    b = 5;
    TEST_ASSERT_TRUE(b == 5);
    bzero(buf, sizeof(buf));
    sprintf(buf, "a: %lf, b: %lf, b type: %d, ram: 0x%x",
            a.getUniversalDouble(),
            b.getUniversalDouble(),
            b.getType(),
            b.getRAM());
    TEST_ASSERT_TRUE_MESSAGE(a <= b, buf);
    a = -8;
    TEST_ASSERT_TRUE(a <= -2);
    a = -2;
    TEST_ASSERT_TRUE(a <= -2);

    a = "a";
    TEST_ASSERT_TRUE(a <= "b");
    TEST_ASSERT_TRUE(a <= "a");

    // >=
    a = 8;
    b = 3;
    TEST_ASSERT_TRUE(a >= 6);
    a = 6;
    TEST_ASSERT_TRUE(a >= 6);
    TEST_ASSERT_TRUE(a >= 5);
    TEST_ASSERT_TRUE(a >= b);
    b = 6;
    bzero(buf, sizeof(buf));
    sprintf(buf, "a: %lf, b: %lf, b type: %d, ram: 0x%x",
            a.getUniversalDouble(),
            b.getUniversalDouble(),
            b.getType(),
            b.getRAM());
    TEST_ASSERT_TRUE_MESSAGE(a >= b, buf);
    a = -8;
    TEST_ASSERT_TRUE(a >= -9);
    a = -2;
    TEST_ASSERT_TRUE(a >= -2);

    a = "c";
    TEST_ASSERT_TRUE(a >= "b");
    TEST_ASSERT_TRUE(a >= "c");

    // +

    a = 2;
    b = 8;
    TEST_ASSERT_TRUE(a + b == 10);
    a = -7;
    TEST_ASSERT_TRUE(a + b == 1);

    a = "Hello";
    b = "World!";

    TEST_ASSERT_EQUAL_STRING("Hello World!", (a + " " + b).c_str());

    a = "c";
    b = 1;

    TEST_ASSERT_EQUAL_STRING("c1", (a + b).c_str());

    // -
    a = 8;
    b = 2;
    TEST_ASSERT_TRUE(a - b == 6);
    TEST_ASSERT_TRUE(a - 5 == 3);
    a = -9;
    TEST_ASSERT_TRUE(a - b == -11);

    a = "HeABCllo ABCWorld!ABC";

    TEST_ASSERT_EQUAL_STRING("Hello World!", (a - "ABC").c_str());

    // *

    a = 2;
    b = 5;
    TEST_ASSERT_TRUE(a * b == 10);
    TEST_ASSERT_TRUE(a * 3 == 6);
    a = -8;
    TEST_ASSERT_TRUE(a * 2 == -16);

    a = "Hello World! ";

    b = 3;

    TEST_ASSERT_EQUAL_STRING("Hello World! Hello World! Hello World! ", (a * b).c_str());

    // /

    a = 10;
    b = 2;

    TEST_ASSERT_TRUE(a / b == 5.0f);

    a = 5;
    TEST_ASSERT_TRUE(a / b == 2);

    a = 7.5f;
    TEST_ASSERT_TRUE(a / 2 == 3);

    a = 7.5f;
    TEST_ASSERT_TRUE(a / 2.0f == 3.75f);

    // ++
    a = 1;
    TEST_ASSERT_TRUE(a++ == 1);
    TEST_ASSERT_TRUE(a == 2);
    TEST_ASSERT_TRUE(++a == 3);

    a = -65542;
    TEST_ASSERT_TRUE(a++ == -65542);
    TEST_ASSERT_TRUE(a == -65541);
    TEST_ASSERT_TRUE(++a == -65540);

    a = (float)1.25f;
    TEST_ASSERT_TRUE(a++ == (float)1.25f);
    TEST_ASSERT_TRUE(a == (float)2.25f);
    TEST_ASSERT_TRUE(++a == (float)3.25f);

    a = (double)1.25f;
    TEST_ASSERT_TRUE(a++ == (double)1.25f);
    TEST_ASSERT_TRUE(a == (double)2.25f);
    TEST_ASSERT_TRUE(++a == (double)3.25f);

    a = "6";
    TEST_ASSERT_TRUE(++a == "66");
    TEST_ASSERT_TRUE(++a == "6666");
    TEST_ASSERT_TRUE(++a == "66666666");

    // --
    a = 10;
    TEST_ASSERT_TRUE(a-- == 10);
    TEST_ASSERT_TRUE(a == 9);
    TEST_ASSERT_TRUE(--a == 8);

    a = -65542;
    TEST_ASSERT_TRUE(a-- == -65542);
    TEST_ASSERT_TRUE(a == -65543);
    TEST_ASSERT_TRUE(--a == -65544);

    a = (float)3.25f;
    TEST_ASSERT_TRUE(a-- == (float)3.25f);
    TEST_ASSERT_TRUE(a == (float)2.25f);
    TEST_ASSERT_TRUE(--a == (float)1.25f);

    a = (double)3.25f;
    TEST_ASSERT_TRUE(a-- == (double)3.25f);
    TEST_ASSERT_TRUE(a == (double)2.25f);
    TEST_ASSERT_TRUE(--a == (double)1.25f);

    // +=
    a = 10;
    a += 10;
    TEST_ASSERT_TRUE(a == 20);

    b = 6;
    a += b;
    TEST_ASSERT_TRUE(a == 26);
    a = (double)19.0f;
    b = (double)0.25f;

    a += b;
    bzero(buf, sizeof(buf));
    sprintf(buf, "%lf", a.getUniversalDouble());
    TEST_ASSERT_TRUE_MESSAGE(a == 19.25f, buf);

    a = (float)1.25f;
    a += 1.25f;
    TEST_ASSERT_TRUE(a == 2.5f);

    a = (double)1.25f;
    a += 1.25f;
    TEST_ASSERT_TRUE(a == 2.5f);

    a = "Hello ";
    a += "World!";

    TEST_ASSERT_EQUAL_STRING("Hello World!", a.c_str());

    // -=
    a = 20;
    a -= 10;
    TEST_ASSERT_TRUE(a == 10);

    b = 2;
    a -= b;
    TEST_ASSERT_TRUE(a == 8);
    a = (double)20.0f;
    b = (double)0.25f;
    a -= b;
    TEST_ASSERT_TRUE(a == 19.75f);

    a = (float)2.25f;
    a -= 1.25f;
    TEST_ASSERT_TRUE(a == 1.0f);

    a = (double)2.25f;
    a -= 1.25f;
    TEST_ASSERT_TRUE(a == 1.0f);

    a = "HeABCllo ABCWorld!ABC";
    a -= "ABC";

    TEST_ASSERT_EQUAL_STRING("Hello World!", a.c_str());

    // *=

    a = 8;
    a *= 8;
    TEST_ASSERT_TRUE(a == 64);

    a = (double)(-8.25f);
    b = a;

    a *= b;
    TEST_ASSERT_TRUE(a == (double)(68.0625f));

    // /=

    a = 64;
    a /= 8;
    TEST_ASSERT_TRUE(a == 8);

    a = (double)(-10.25f);
    b = (double)2.0f;

    a /= b;
    bzero(buf, sizeof(buf));
    sprintf(buf, "%lf", a.getUniversalDouble());
    TEST_ASSERT_TRUE_MESSAGE(a == (double)(-5.125f), buf);

    // <<

    a = 2;
    TEST_ASSERT_TRUE(a << 2 == 8);

    b = 2;
    a = 2;
    TEST_ASSERT_TRUE(a << b == 8);

    a = "abcde";
    TEST_ASSERT_TRUE(a << b == "cde");

    // >>
    a = 8;
    TEST_ASSERT_TRUE(a >> 2 == 2);

    b = 2;
    a = 8;
    TEST_ASSERT_TRUE(a >> b == 2);

    a = "abcde";
    TEST_ASSERT_TRUE(a >> b == "abc");

    // %
    a = 10;
    b = 3;
    TEST_ASSERT_TRUE(a % b == 1);

    // %=

    a = 100;
    b = 13;
    a %= b;
    TEST_ASSERT_TRUE(a == 9);

    // <<=
    a = 2;
    a <<= 2;
    TEST_ASSERT_TRUE(a == 8);

    b = 2;
    a = 2;
    a <<= b;
    TEST_ASSERT_TRUE(a == 8);

    a = "abcde";
    a <<= b;
    TEST_ASSERT_TRUE(a == "cde");

    // >>=
    a = 8;
    a >>= 2;
    TEST_ASSERT_TRUE(a == 2);

    b = 2;
    a = 8;
    a >>= b;
    TEST_ASSERT_TRUE(a == 2);

    a = "abcde";
    a >>= b;
    TEST_ASSERT_TRUE(a == "abc");

    // &
    a = 0b01001000;
    b = 0b00101000;
    u8 = 0b00101000;
    Element c = 0b00100000;

    TEST_ASSERT_TRUE((a & u8) == 0b00001000);
    TEST_ASSERT_TRUE((a & b) == 0b00001000);
    TEST_ASSERT_TRUE((a & c) == 0);

    // |
    a = 0b01001000;
    b = 0b00101000;
    u8 = 0b00101000;
    c = 0b00100001;

    TEST_ASSERT_TRUE((a | u8) == 0b01101000);
    TEST_ASSERT_TRUE((a | b) == 0b01101000);
    TEST_ASSERT_TRUE((a | c) == 0b01101001);

    // ~
    a = 0xFFFF;
    TEST_ASSERT_TRUE(((~a) & 0xffff) == 0x0);

    // &=
    a = 0b01001000;
    b = 0b00101000;
    a &= b;

    TEST_ASSERT_TRUE(a == 0b00001000);

    // |=
    a = 0b01001000;
    b = 0b00101000;
    a &= b;

    TEST_ASSERT_TRUE(a == 0b00001000);

    // ^
    a = 0b10000100;
    b = 0b10111111;

    TEST_ASSERT_TRUE((a ^ b) == 0b00111011);

    // ^=
    a = 0b10000100;
    b = 0b10111111;
    a ^= b;
    TEST_ASSERT_TRUE(a == 0b00111011);

    // &&
    a = 0;
    TEST_ASSERT_TRUE((a && false) == false);

    a = (float)0.0f;
    TEST_ASSERT_TRUE((a && false) == false);

    a = (float)1.23f;
    TEST_ASSERT_TRUE((a && true) == true);

    a = (double)0.0f;
    TEST_ASSERT_TRUE((a && false) == false);

    a = (double)3.1415926f;
    TEST_ASSERT_TRUE((a && true) == true);

    a = "abc";
    TEST_ASSERT_TRUE((a && false) == false);

    a = "";
    TEST_ASSERT_TRUE((a && false) == false);

    a = 666;
    TEST_ASSERT_TRUE(a && true);

    // ||
    a = 0;
    TEST_ASSERT_TRUE((a || false) == false);

    a = (float)0.0f;
    TEST_ASSERT_TRUE((a || true) == true);

    a = (float)1.23f;
    TEST_ASSERT_TRUE((a || false) == true);

    a = (double)0.0f;
    TEST_ASSERT_TRUE((a || true) == true);

    a = (double)3.1415926f;
    TEST_ASSERT_TRUE((a || false) == true);

    a = "abc";
    TEST_ASSERT_TRUE((a || false) == true);

    a = "";
    bzero(buf, sizeof(buf));
    sprintf(buf, "string length: %u, type: 0x%x, str: [%s]", strlen(a.c_str()), a.getType(), a.c_str());
    TEST_ASSERT_TRUE_MESSAGE((a || false) == false, buf);

    a = 666;
    TEST_ASSERT_TRUE(a || true);

    // !
    a = 6;
    TEST_ASSERT_TRUE(!a == false);
    a = 0;
    bzero(buf, sizeof(buf));
    sprintf(buf, "type: %d, value: %llu, buffer length: %u", a.getType(), a.getUint64(), a.getRawBufferLength());
    TEST_ASSERT_TRUE_MESSAGE(!a == true, buf);

    a = (float)0.0f;
    TEST_ASSERT_TRUE(!a == true);

    a = (float)1.23f;
    TEST_ASSERT_TRUE(!a == false);

    a = (double)0.0f;
    TEST_ASSERT_TRUE(!a == true);

    a = (double)3.1415926f;
    TEST_ASSERT_TRUE(!a == false);

    a = "";
    bzero(buf, sizeof(buf));
    sprintf(buf, "string length: %u, type: 0x%x, str: [%s]", strlen(a.c_str()), a.getType(), a.c_str());
    TEST_ASSERT_TRUE_MESSAGE(!a == true, buf);

    a = "abc";
    TEST_ASSERT_TRUE(!a == false);

    // []

    a = "Hello World!";

    String str = "";
    str += (char)a[6];
    TEST_ASSERT_EQUAL_STRING("W", str.c_str());

    bzero(buf, sizeof(buf));
    sprintf(buf, "[]value: %d", a[12]);

    TEST_ASSERT_TRUE_MESSAGE(a[12] == 0, buf);

    TEST_ASSERT_TRUE_MESSAGE(a[13] < 0, buf);

    // 2023/03/23 all tests passed
}

void test_element_getHex()
{
    uint8_t buffer[] = {0, 0x80, 0xff};
    Element a(buffer, 3);
    String result = a.getHex();
    TEST_ASSERT_TRUE(result == "0080ff");
}

void test_element_convertHexStringIntoUint8Array()
{
    Element a("0080ff");
    TEST_ASSERT_TRUE(a.convertHexStringIntoUint8Array());
    uint8_t *buffer = a.getUint8Array();
    uint8_t value[] = {0, 0x80, 0xff};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(value, buffer, 3);
}

void test_createArrayBuffer_and_decodeArrayBuffer()
{

    uint8_t buffer[1024] = {0};
    for (uint32_t i = 0; i < 1024; ++i)
    {
        buffer[i] = random(0, 0xff);
    }
    std::vector<Element *> container =
        {
            new Element(0x01),
            new Element(0x80),
            new Element(0xffff),
            new Element((int)0xffffffff),
            new Element(-8),
            new Element(-160),
            new Element(-40000),
            new Element(-2147483666),
            new Element((float)1.23f),
            new Element((double)3.1415926f),
            new Element("Hello world!"),
            new Element(buffer, (uint32_t)1024)};

    // directly use callback version, it included normal version
    ArrayBuffer::createArrayBuffer(
        &container,
        [&container](uint8_t *output, uint64_t length, bool *isBufferDeleted)
        {
            ArrayBuffer::decodeArrayBuffer(
                [&container](std::vector<Element *> *decoded)
                {
                    TEST_ASSERT_TRUE(decoded->size() == container.size());
                    for (int i = 0; i < container.size(); ++i)
                    {

                        // char *buf = new char[1024];
                        // bzero(buf, 1024);
                        // sprintf(buf,
                        //         "index: %u, origin type:%d, decoded type: %d, string orgin: %s, string decoded: %s, double orgin: %lf, double decoded: %lf, string A length: %lu, string B length: %lu",
                        //         i,
                        //         container.at(i)->getType(),
                        //         decoded->at(i)->getType(),
                        //         container.at(i)->getString().c_str(),
                        //         decoded->at(i)->getString().c_str(),
                        //         container.at(i)->getUniversalDouble(),
                        //         decoded->at(i)->getUniversalDouble(),
                        //         container.at(i)->getRawBufferLength(),
                        //         decoded->at(i)->getRawBufferLength());
                        TEST_ASSERT_TRUE_MESSAGE(
                            (*(decoded->at(i))) == (*(container.at(i))),
                            "");
                    }
                },
                output,
                length);
        });
}

void test_element_SHA()
{
    Element a = "Hello World!";
    Element sha = "7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069";
    String b = a.getSHA256HexString(true);
    TEST_ASSERT_TRUE_MESSAGE(
        a == sha,
        b.c_str());
}

void test_element_crypto()
{
    Element a = "Hello World!";

    TEST_ASSERT_TRUE(a.AES256_CBC("0123456789abcdef0123456789abcdef", "0123456789abcdef"));

    TEST_ASSERT_TRUE_MESSAGE(a.getHex() == "68c389f8fe135cb9fda801754c36b361", a.getHex().c_str());

    a = "Hello World!";
    TEST_ASSERT_TRUE(a.toAES256CBCHexString("0123456789abcdef0123456789abcdef", "0123456789abcdef"));
    TEST_ASSERT_TRUE_MESSAGE(a == "68c389f8fe135cb9fda801754c36b361", a.c_str());

    a = "hello world!";
    TEST_ASSERT_TRUE(a.toAES256CBCHexString("0123456789abcdef0123456789abcdef", "0123456789abcdef"));
    TEST_ASSERT_TRUE_MESSAGE(a == "700d435bf4c1ff6fbce7f8af99e8c1a9", a.c_str());

    a = "Hello World!";
    TEST_ASSERT_TRUE(a.toAES256CBCBase64("0123456789abcdef0123456789abcdef", "0123456789abcdef"));
    TEST_ASSERT_TRUE_MESSAGE(a == "aMOJ+P4TXLn9qAF1TDazYQ==", a.c_str());

    // base64
    a = "Hello World!";
    a.toBase64(true);
    TEST_ASSERT_TRUE(a == "SGVsbG8gV29ybGQh");
    a.fromBase64(true);
    TEST_ASSERT_TRUE(a == "Hello World!");

    // 2023/03/24, all tests passed
    /*
    -----------------------------------------------------------------------------------------------------------------------------------------
    Building & Uploading...
    Testing...
    If you don't see any output for the first 10 secs, please reset board (press reset button)

    test/test.cpp:735: test_element_new_operators   [PASSED]
    test/test.cpp:736: test_element_getHex  [PASSED]
    test/test.cpp:737: test_element_convertHexStringIntoUint8Array  [PASSED]
    test/test.cpp:738: test_createArrayBuffer_and_decodeArrayBuffer [PASSED]
    test/test.cpp:739: test_element_SHA     [PASSED]
    test/test.cpp:740: test_element_crypto  [PASSED]
    test/test.cpp:743: test_sha1    [PASSED]
    test/test.cpp:744: test_sha256  [PASSED]
    test/test.cpp:745: test_base64_encode   [PASSED]
    test/test.cpp:746: test_base64_decode   [PASSED]
    test/test.cpp:747: test_aes_encode      [PASSED]
    test/test.cpp:748: test_aes_decode      [PASSED]
    test/test.cpp:751: test_mydb    [PASSED]
    -------------------------------------------------- esp32:* [PASSED] Took 15.02 seconds --------------------------------------------------

    ================================================================ SUMMARY ================================================================
    Environment    Test    Status    Duration
    -------------  ------  --------  ------------
    esp32          *       PASSED    00:00:15.020
    ============================================== 13 test cases: 13 succeeded in 00:00:15.020 ==============================================
    */
}

void test_mydb()
{
    MyFS::myfsInit();
    MyDB db_unit_test("db_unit_test");
    db_unit_test.begin();
    TEST_ASSERT_FALSE(db_unit_test("key")->available());
    *db_unit_test("key") = 100;
    TEST_ASSERT_TRUE(*db_unit_test("key") == 100);

    TEST_ASSERT_TRUE(db_unit_test.flush());

    db_unit_test.unload();

    db_unit_test.begin();
    TEST_ASSERT_TRUE(*db_unit_test("key") == 100);

    TEST_ASSERT_TRUE(MyFS::fileExist("db_unit_test.db"));

    TEST_ASSERT_TRUE(db_unit_test.unloadAndRemoveFile("db_unit_test"));

    TEST_ASSERT_FALSE(MyFS::fileExist("db_unit_test.db"));
    TEST_ASSERT_FALSE(MyFS::fileExist("db_unit_test.bak"));
}

void setup()
{
    mycrypto::SHA::initialize();
    mycrypto::AES::initialize();
    delay(1000);
    UNITY_BEGIN();

    // array buffer
    RUN_TEST(test_element_new_operators);
    RUN_TEST(test_element_getHex);
    RUN_TEST(test_element_convertHexStringIntoUint8Array);
    RUN_TEST(test_createArrayBuffer_and_decodeArrayBuffer);
    RUN_TEST(test_element_SHA);
    RUN_TEST(test_element_crypto);

    // mycrypto
    RUN_TEST(test_sha1);
    RUN_TEST(test_sha256);
    RUN_TEST(test_base64_encode);
    RUN_TEST(test_base64_decode);
    RUN_TEST(test_aes_encode);
    RUN_TEST(test_aes_decode);

    // mydb
    RUN_TEST(test_mydb);

    UNITY_END();
}
void loop()
{
}
