#include <Arduino.h>
#include <unity.h>
#include <arraybuffer.h>
#include <vector>
#include <mycrypto.h>
#include <mydb.h>

void test_sha1()
{
    String a = "hello world!";
    String result = "430ce34d020724ed75a196dfc2ad67c77772d169";

    TEST_ASSERT_EQUAL_STRING(result.c_str(), mycrypto::SHA::sha1(a).c_str());
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
    String result = "700d435bf4c1ff6fbce7f8af99e8c1a9";
    TEST_ASSERT_EQUAL_STRING(result.c_str(), mycrypto::AES::aes256CBCEncrypt(key, iv, plain).c_str());
}

void test_aes_decode()
{
    String key = "0123456789abcdef0123456789abcdef";
    String iv = "0123456789abcdef";

    String cipher = "700d435bf4c1ff6fbce7f8af99e8c1a9";
    String result = "hello world!";
    TEST_ASSERT_EQUAL_STRING(result.c_str(), mycrypto::AES::aes256CBCDecrypt(key, iv, cipher).c_str());
}

void test_element_constrauctors_and_operators()
{
    Element *z = new Element();
    TEST_ASSERT_NULL(z->getRawBuffer());
    delete z;

    z = new Element("Hello world!");
    TEST_ASSERT_NOT_NULL(z->getRawBuffer());
    delete z;

    Element a(0xff);
    Element b((uint8_t)0xff);
    Element *c = new Element(0xff);
    Element d((uint16_t)0xffff);
    Element e((uint64_t)0xffffffff);

    uint8_t buffer[] = {1, 2, 3};
    Element x(buffer, 3);
    Element y(buffer, 3);

    TEST_ASSERT_TRUE(a == b);
    TEST_ASSERT_TRUE(!(a == d));
    TEST_ASSERT_TRUE(!(a == e));
    TEST_ASSERT_TRUE(a.equalsTo(&b));
    TEST_ASSERT_TRUE(a.equalsTo(c));

    TEST_ASSERT_TRUE(a < d);
    TEST_ASSERT_TRUE(d > a);

    TEST_ASSERT_TRUE(a < &d);
    TEST_ASSERT_TRUE(d > &a);

    TEST_ASSERT_TRUE(a <= d);
    TEST_ASSERT_TRUE(d >= a);

    TEST_ASSERT_TRUE(a <= &d);
    TEST_ASSERT_TRUE(d >= &a);

    TEST_ASSERT_TRUE(x.getU8aLen() == 3);
    TEST_ASSERT_TRUE(x == y);

    a.setNumber(100);
    TEST_ASSERT_TRUE(a.getNumber() == 100);

    a.setString(String("Hello world!"));
    TEST_ASSERT_TRUE(a.getType() == STRING);
    TEST_ASSERT_TRUE(a.getString() == "Hello world!");

    TEST_ASSERT_TRUE(a.setUint8Array(buffer, 3));
    TEST_ASSERT_TRUE(a.getType() == U8A);
    TEST_ASSERT_TRUE(a.getU8aLen() == 3);
    TEST_ASSERT_TRUE(a.getUint8Array()[2] == 3);

    int number = random(0, 0xff);

    a = number;
    TEST_ASSERT_TRUE(a.getNumber() == number);

    number = random(0xff, 0xffff);
    a = number;
    TEST_ASSERT_TRUE(a.getNumber() == number);

    number = random(0xffff, 0xffffffff);
    a = number;
    TEST_ASSERT_TRUE(a.getNumber() == number);

    String helloWorld = "Hello world!";
    a = helloWorld.c_str();
    TEST_ASSERT_EQUAL_STRING(helloWorld.c_str(), a.getString().c_str());

    a = 0;

    a = helloWorld;
    TEST_ASSERT_EQUAL_STRING(helloWorld.c_str(), a.getString().c_str());

    a = 0;

    a = &helloWorld;
    TEST_ASSERT_EQUAL_STRING(helloWorld.c_str(), a.getString().c_str());

    a = d;
    TEST_ASSERT_TRUE(a == d);

    a = c;
    TEST_ASSERT_TRUE(a == c);

    delete c;
}

void test_element_math_operators()
{
    // unity tool doest not support 64bits number some condition
    // so use 32bits

    // +
    Element a;
    Element b = 90;
    a = 6;
    TEST_ASSERT_TRUE((uint32_t)(a + 10) == (uint32_t)(6 + 10));
    TEST_ASSERT_TRUE((uint32_t)(a + b) == (uint32_t)(6 + 90));
    a += 2;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(8));

    // -
    a = 10;
    b = 1;
    TEST_ASSERT_TRUE((uint32_t)(a - 1) == (uint32_t)(10 - 1));
    TEST_ASSERT_TRUE((uint32_t)(a - b) == (uint32_t)(10 - 1));
    a -= 2;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(8));

    // *, *=
    a = 10;
    b = 100;
    TEST_ASSERT_TRUE((uint32_t)(a * 100) == (uint32_t)(10 * 100));
    TEST_ASSERT_TRUE((uint32_t)(a * b) == (uint32_t)(10 * 100));
    a *= 100;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(1000));
    a = 10;
    a *= b;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(1000));

    // /, /=
    a = 1000;
    b = 10;
    TEST_ASSERT_TRUE((uint32_t)(a / 10) == (uint32_t)(1000 / 10));
    TEST_ASSERT_TRUE((uint32_t)(a / b) == (uint32_t)(1000 / 10));
    a /= 10;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(100));
    a = 1000;
    a /= b;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(100));

    // %, %=
    a = 96;
    b = 9;
    TEST_ASSERT_TRUE((uint32_t)(a % 9) == (uint32_t)(96 % 9));
    TEST_ASSERT_TRUE((uint32_t)(a % b) == (uint32_t)(96 % 9));
    a %= 9;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(6));

    // String +=
    a = "hahaha";
    a += "666";
    TEST_ASSERT_EQUAL_STRING("hahaha666", a.c_str());

    // ++
    a = 10;

    TEST_ASSERT_TRUE((a++).getNumber() == 10);
    TEST_ASSERT_TRUE((++a).getNumber() == 12);

    // --
    a = 10;

    TEST_ASSERT_TRUE((a--).getNumber() == 10);
    TEST_ASSERT_TRUE((--a).getNumber() == 8);

    // <<
    a = 8;
    TEST_ASSERT_TRUE((uint32_t)(a << 1) == (uint32_t)(8 << 1));
    a <<= 1;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(8 << 1));

    // >>
    a = 16;
    TEST_ASSERT_TRUE((uint32_t)(a >> 1) == (uint32_t)(16 >> 1));
    a >>= 1;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(16 >> 1));

    // ^
    a = 6;
    TEST_ASSERT_TRUE((uint32_t)(a ^ 10) == (uint32_t)(6 ^ 10));
    a ^= 10;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(6 ^ 10));

    // &
    a = 6;
    TEST_ASSERT_TRUE((uint32_t)(a & 10) == (uint32_t)(6 & 10));

    a &= 10;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(6 & 10));

    // |
    a = 6;
    TEST_ASSERT_TRUE((uint32_t)(a | 10) == (uint32_t)(6 | 10));
    a |= 10;
    TEST_ASSERT_TRUE((uint32_t)a.getNumber() == (uint32_t)(6 | 10));

    // ~
    a = 6;
    TEST_ASSERT_TRUE((uint32_t)(~a) == (uint32_t)(~6));
}

void test_element_copyFrom()
{
    uint8_t buffer[] = {0, 0x80, 0xff};
    String helloWorld = "Hello world!";
    Element b(helloWorld);
    int n = random(0, 0xffff);

    Element a(n);

    a.copyFrom(buffer, 3);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(a.getUint8Array(), buffer, 3);

    a.copyFrom(&b);
    TEST_ASSERT_EQUAL_STRING(a.getString().c_str(), b.getString().c_str());

    TEST_ASSERT_TRUE(a.getString().length() == b.getString().length());

    b.setNumber(n);
    a.copyFrom(&b);
    TEST_ASSERT_TRUE(a.getNumber() == n);
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
            new Element(0xffffffff),
            new Element("Hello world!"),
            new Element(buffer, 1024)};

    // directly use callback version, it included normal version
    ArrayBuffer::createArrayBuffer(
        &container,
        [&container](uint8_t *output, uint64_t length, bool *isBufferDeleted)
        {
            ArrayBuffer::decodeArrayBuffer(
                [&container](std::vector<Element *> *origin)
                {
                    TEST_ASSERT_TRUE(origin->size() == 6);
                    for (int i = 0; i < 6; ++i)
                    {
                        // char buf[1024] = {0};
                        // sprintf(buf, "index: %u, typeA: %u, typeB: %u, stringA: %s, stringB: %s",
                        //         i, origin->at(i)->getType(), container.at(i)->getType(),
                        //         origin->at(i)->getString().c_str(),
                        //         container.at(i)->getString().c_str());
                        TEST_ASSERT_TRUE_MESSAGE(*(origin->at(i)) == *(container.at(i)), "");
                    }
                },
                output,
                length);
        });
}

void test_element_type()
{
    Element a = 10;
    TEST_ASSERT_TRUE(a.getType(true) == NUMBER);
    TEST_ASSERT_TRUE(a.getType() == UINT8);

    a = (uint8_t)10;
    TEST_ASSERT_TRUE(a.getType(true) == NUMBER);
    TEST_ASSERT_TRUE(a.getType() == UINT8);

    a = (uint16_t)65535;
    TEST_ASSERT_TRUE(a.getType(true) == NUMBER);
    TEST_ASSERT_TRUE(a.getType() == UINT16);

    a = (uint32_t)65536;
    TEST_ASSERT_TRUE(a.getType(true) == NUMBER);
    TEST_ASSERT_TRUE(a.getType() == UINT32);

    a = (uint64_t)0xffffffffffffULL;
    TEST_ASSERT_TRUE(a.getType(true) == NUMBER);
    TEST_ASSERT_TRUE(a.getType() == UINT64);

    a = "Hello world";
    TEST_ASSERT_TRUE(a.getType(true) != NUMBER);
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
    RUN_TEST(test_element_constrauctors_and_operators);
    RUN_TEST(test_element_math_operators);
    RUN_TEST(test_element_copyFrom);
    RUN_TEST(test_element_getHex);
    RUN_TEST(test_element_type);
    RUN_TEST(test_element_convertHexStringIntoUint8Array);
    RUN_TEST(test_createArrayBuffer_and_decodeArrayBuffer);

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
