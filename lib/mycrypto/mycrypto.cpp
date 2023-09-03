#include "mycrypto.h"

namespace mycrypto
{
    // universal sha digest function
    // use esp32 built-in hardware acceleration module
    // all procedures is from Espressif offical technical document
    // befor using this function, you need call "periph_module_enable(PERIPH_SHA_MODULE);" at first
    // this is included in this class
    // call SHA::initialize();
    // or you will got all zero result

#if CONFIG_IDF_TARGET_ESP32S3
    void SHA::sha(uint8_t *data, uint64_t length, uint32_t *output, SHAType type)
    {
        // type 1 for sha1
        // 0 for sha256
        if (length <= 0)
        {
            bzero(output, (type & 1 ? 5 : 8));
            return;
        }

        // original length
        uint64_t ori = length;

        // calc padding length
        length = ((length * 8) % 512);
        uint64_t zeroLength = ((length < 448) ? (448 - length) : (448 + 512 - length)) / 8;

        // add length
        length = ori + zeroLength + 8;

        // allocate buffer
        uint8_t *buf = new (std::nothrow) uint8_t[length];

        if (!buf)
        {
            output[0] = 0;
            return;
        }

        // padding zero
        bzero(buf, length);

        // copy original data
        memcpy(buf, data, ori);

        // padding the "1" after data
        buf[ori] = (uint8_t)0x80;

        // add data length(bits) to the tail
        uint64_t bits = ori * 8;
        for (int i = 0; i < 8; i++)
        {
            buf[ori + zeroLength + i] = (bits >> ((7 - i) * 8)) & 0xff;
        }

        uint64_t i = 0;

        if (type & 1)
        {
            DPORT_REG_WRITE(SHA_MODE_REG, (uint32_t)0);
        }
        else
        {
            DPORT_REG_WRITE(SHA_MODE_REG, (uint32_t)2);
        }

        // fill 512 bits(1 block) to start
        DPORT_REG_WRITE(SHA_TEXT_BASE + 0, TO_32_LE(buf));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 4, TO_32_LE(buf + 4));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 8, TO_32_LE(buf + 8));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 12, TO_32_LE(buf + 12));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 16, TO_32_LE(buf + 16));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 20, TO_32_LE(buf + 20));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 24, TO_32_LE(buf + 24));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 28, TO_32_LE(buf + 28));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 32, TO_32_LE(buf + 32));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 36, TO_32_LE(buf + 36));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 40, TO_32_LE(buf + 40));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 44, TO_32_LE(buf + 44));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 48, TO_32_LE(buf + 48));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 52, TO_32_LE(buf + 52));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 56, TO_32_LE(buf + 56));
        DPORT_REG_WRITE(SHA_TEXT_BASE + 60, TO_32_LE(buf + 60));
        i += 64;

        // start
        if (type & 1)
        {
            DPORT_REG_WRITE(SHA_START_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_BUSY_REG))
            {
                // yield();
            }
        }
        else
        {
            DPORT_REG_WRITE(SHA_START_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_BUSY_REG))
            {
            }
        }

        // to process other blocks
        // always fill 512bits(a block) at one time
        for (; i < length; i += 64)
        {

            // fill 512 bits(1 block) to start
            DPORT_REG_WRITE(SHA_TEXT_BASE + 0, TO_32_LE(buf + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 4, TO_32_LE(buf + 4 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 8, TO_32_LE(buf + 8 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 12, TO_32_LE(buf + 12 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 16, TO_32_LE(buf + 16 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 20, TO_32_LE(buf + 20 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 24, TO_32_LE(buf + 24 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 28, TO_32_LE(buf + 28 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 32, TO_32_LE(buf + 32 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 36, TO_32_LE(buf + 36 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 40, TO_32_LE(buf + 40 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 44, TO_32_LE(buf + 44 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 48, TO_32_LE(buf + 48 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 52, TO_32_LE(buf + 52 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 56, TO_32_LE(buf + 56 + i));
            DPORT_REG_WRITE(SHA_TEXT_BASE + 60, TO_32_LE(buf + 60 + i));

            // continue
            if (type & 1)
            {
                DPORT_REG_WRITE(SHA_CONTINUE_REG, (uint32_t)(1));

                while (DPORT_REG_READ(SHA_BUSY_REG))
                {
                }
            }
            else
            {
                DPORT_REG_WRITE(SHA_CONTINUE_REG, (uint32_t)(1));

                while (DPORT_REG_READ(SHA_BUSY_REG))
                {
                }
            }
        }
        delete buf;

        while (DPORT_REG_READ(SHA_BUSY_REG))
        {
        }

        uint8_t shaLen = type & 1 ? 5 : 8;

        for (int i = 0; i < shaLen; i++)
        {
            output[i] = (uint32_t)DPORT_REG_READ(SHA_H_BASE + (i * 4));
            output[i] = (uint32_t)(((uint32_t)(output[i] & 0x000000ffu) << 24) |
                                   ((uint32_t)(output[i] & 0x0000ff00u) << 8) |
                                   ((uint32_t)(output[i] & 0x00ff0000u) >> 8) |
                                   ((uint32_t)(output[i] & 0xff000000u) >> 24));
        }
    }
#elif CONFIG_IDF_TARGET_ESP32
    void SHA::sha(uint8_t *data, uint64_t length, uint32_t *output, SHAType type)
    {
        // type 1 for sha1
        // 0 for sha256
        if (length <= 0)
        {
            bzero(output, (type & 1 ? 5 : 8));
            return;
        }

        // original length
        uint64_t ori = length;

        // calc padding length
        length = ((length * 8) % 512);
        uint64_t zeroLength = ((length < 448) ? (448 - length) : (448 + 512 - length)) / 8;

        // add length
        length = ori + zeroLength + 8;

        // allocate buffer
        uint8_t *buf = new (std::nothrow) uint8_t[length];

        if (!buf)
        {
            output[0] = 0;
            return;
        }

        // padding zero
        bzero(buf, length);

        // copy original data
        memcpy(buf, data, ori);

        // padding the "1" after data
        buf[ori] = (uint8_t)0x80;

        // add data length(bits) into the tail
        uint64_t bits = ori * 8;
        for (int i = 0; i < 8; i++)
        {
            buf[ori + zeroLength + i] = (bits >> ((7 - i) * 8)) & 0xff;
        }

        uint64_t i = 0;

        // fill 512 bits(1 block) to start
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 0, (uint32_t)((buf[i + 0] << 24) + (buf[i + 1] << 16) + (buf[i + 2] << 8) + (buf[i + 3])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 4, (uint32_t)((buf[i + 4] << 24) + (buf[i + 5] << 16) + (buf[i + 6] << 8) + (buf[i + 7])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 8, (uint32_t)((buf[i + 8] << 24) + (buf[i + 9] << 16) + (buf[i + 10] << 8) + (buf[i + 11])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 12, (uint32_t)((buf[i + 12] << 24) + (buf[i + 13] << 16) + (buf[i + 14] << 8) + (buf[i + 15])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 16, (uint32_t)((buf[i + 16] << 24) + (buf[i + 17] << 16) + (buf[i + 18] << 8) + (buf[i + 19])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 20, (uint32_t)((buf[i + 20] << 24) + (buf[i + 21] << 16) + (buf[i + 22] << 8) + (buf[i + 23])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 24, (uint32_t)((buf[i + 24] << 24) + (buf[i + 25] << 16) + (buf[i + 26] << 8) + (buf[i + 27])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 28, (uint32_t)((buf[i + 28] << 24) + (buf[i + 29] << 16) + (buf[i + 30] << 8) + (buf[i + 31])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 32, (uint32_t)((buf[i + 32] << 24) + (buf[i + 33] << 16) + (buf[i + 34] << 8) + (buf[i + 35])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 36, (uint32_t)((buf[i + 36] << 24) + (buf[i + 37] << 16) + (buf[i + 38] << 8) + (buf[i + 39])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 40, (uint32_t)((buf[i + 40] << 24) + (buf[i + 41] << 16) + (buf[i + 42] << 8) + (buf[i + 43])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 44, (uint32_t)((buf[i + 44] << 24) + (buf[i + 45] << 16) + (buf[i + 46] << 8) + (buf[i + 47])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 48, (uint32_t)((buf[i + 48] << 24) + (buf[i + 49] << 16) + (buf[i + 50] << 8) + (buf[i + 51])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 52, (uint32_t)((buf[i + 52] << 24) + (buf[i + 53] << 16) + (buf[i + 54] << 8) + (buf[i + 55])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 56, (uint32_t)((buf[i + 56] << 24) + (buf[i + 57] << 16) + (buf[i + 58] << 8) + (buf[i + 59])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 60, (uint32_t)((buf[i + 60] << 24) + (buf[i + 61] << 16) + (buf[i + 62] << 8) + (buf[i + 63])));
        i += 64;

        // start
        if (type & 1)
        {
            DPORT_REG_WRITE(SHA_1_START_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_1_BUSY_REG))
            {
                // yield();
                // because of the hardware acceleration is very fast
                // for 8KB data only needs less than 300us(ESPRESSIF YYDS)
                // so yield() is no need to call
            }
        }
        else
        {
            DPORT_REG_WRITE(SHA_256_START_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_256_BUSY_REG))
            {
            }
        }

        // to process other blocks
        // always fill 512bits(a block) at one time
        for (; i < length; i += 64)
        {
            // fill 512 bits into registers to continue
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 0, (uint32_t)((buf[i + 0] << 24) + (buf[i + 1] << 16) + (buf[i + 2] << 8) + (buf[i + 3])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 4, (uint32_t)((buf[i + 4] << 24) + (buf[i + 5] << 16) + (buf[i + 6] << 8) + (buf[i + 7])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 8, (uint32_t)((buf[i + 8] << 24) + (buf[i + 9] << 16) + (buf[i + 10] << 8) + (buf[i + 11])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 12, (uint32_t)((buf[i + 12] << 24) + (buf[i + 13] << 16) + (buf[i + 14] << 8) + (buf[i + 15])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 16, (uint32_t)((buf[i + 16] << 24) + (buf[i + 17] << 16) + (buf[i + 18] << 8) + (buf[i + 19])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 20, (uint32_t)((buf[i + 20] << 24) + (buf[i + 21] << 16) + (buf[i + 22] << 8) + (buf[i + 23])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 24, (uint32_t)((buf[i + 24] << 24) + (buf[i + 25] << 16) + (buf[i + 26] << 8) + (buf[i + 27])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 28, (uint32_t)((buf[i + 28] << 24) + (buf[i + 29] << 16) + (buf[i + 30] << 8) + (buf[i + 31])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 32, (uint32_t)((buf[i + 32] << 24) + (buf[i + 33] << 16) + (buf[i + 34] << 8) + (buf[i + 35])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 36, (uint32_t)((buf[i + 36] << 24) + (buf[i + 37] << 16) + (buf[i + 38] << 8) + (buf[i + 39])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 40, (uint32_t)((buf[i + 40] << 24) + (buf[i + 41] << 16) + (buf[i + 42] << 8) + (buf[i + 43])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 44, (uint32_t)((buf[i + 44] << 24) + (buf[i + 45] << 16) + (buf[i + 46] << 8) + (buf[i + 47])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 48, (uint32_t)((buf[i + 48] << 24) + (buf[i + 49] << 16) + (buf[i + 50] << 8) + (buf[i + 51])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 52, (uint32_t)((buf[i + 52] << 24) + (buf[i + 53] << 16) + (buf[i + 54] << 8) + (buf[i + 55])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 56, (uint32_t)((buf[i + 56] << 24) + (buf[i + 57] << 16) + (buf[i + 58] << 8) + (buf[i + 59])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 60, (uint32_t)((buf[i + 60] << 24) + (buf[i + 61] << 16) + (buf[i + 62] << 8) + (buf[i + 63])));

            // continue
            if (type & 1)
            {
                DPORT_REG_WRITE(SHA_1_CONTINUE_REG, (uint32_t)(1));

                while (DPORT_REG_READ(SHA_1_BUSY_REG))
                {
                }
            }
            else
            {
                DPORT_REG_WRITE(SHA_256_CONTINUE_REG, (uint32_t)(1));

                while (DPORT_REG_READ(SHA_256_BUSY_REG))
                {
                }
            }
        }
        delete buf;

        // get sha result
        if (type & 1)
        {
            DPORT_REG_WRITE(SHA_1_LOAD_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_1_BUSY_REG))
            {
            }
        }
        else
        {
            DPORT_REG_WRITE(SHA_256_LOAD_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_256_BUSY_REG))
            {
            }
        }

        uint8_t shaLen = type & 1 ? 5 : 8;

        // read result
        for (int i = 0; i < shaLen; i++)
        {
            output[i] = (uint32_t)DPORT_REG_READ(DR_REG_SHA_BASE + (i * 4));
        }
    }
#endif

    // this is for arduino framework
    String SHA::aSHA(uint8_t *data, uint64_t length, SHAType type, SHAOutputCase hexCase)
    {
        if (!length || !data)
        {
            return "";
        }

        // for sha1 is 160 bits which is 5x32 bits
        // for sha256 is 256 bits which is 8x32 bits
        uint8_t shaLen = type & 1 ? 5 : 8;
        uint32_t output[shaLen];

        // call sha
        sha(data, length, output, type);

        // to store formated hex string
        char hex[9];
        bzero(hex, 9);

        // return value
        String res = "";

        // format
        char format[] = "%08x";

        // case
        if (hexCase == UPPER_CASE)
        {
            format[3] = 'X';
        }

        // convert result into hex string
        for (int i = 0; i < shaLen; i++)
        {
            sprintf(hex, format, output[i]);
            res += hex;
            bzero(hex, 9);
        }
        return res;
    }

    void SHA::convertU32ToU8(uint8_t *data, uint64_t length, uint8_t *output, SHAType type)
    {
        int len = type & 1 ? 5 : 8;
        uint32_t o[len];
        sha(data, length, o, type);
        int k = 0;
        for (int i = 0; i < len; i++)
        {
            output[k++] = (uint8_t)((o[i] & (uint32_t)(0xff000000)) >> 24);
            output[k++] = (uint8_t)((o[i] & (uint32_t)(0x00ff0000)) >> 16);
            output[k++] = (uint8_t)((o[i] & (uint32_t)(0x0000ff00)) >> 8);
            output[k++] = (uint8_t)(o[i] & (uint32_t)(0x000000ff));
        }
    }

    // a very simple base64 encode method
    char *Base64::base64Encode(uint8_t *data, uint64_t length)
    {
        if (!data || !length)
        {
            return nullptr;
        }
        const char *base64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        uint32_t group = 0;

        uint8_t extra = length % 3;

        uint64_t len = length - extra;

        char *res = new (std::nothrow) char[extra == 0 ? (len / 3 * 4 + 1) : (len / 3 * 4 + 5)]{0};

        uint64_t location = 0;
        for (uint64_t i = 0; i < len; i += 3, location += 4)
        {
            group = (data[i + 0] << 24) + (data[i + 1] << 16) + (data[i + 2] << 8);

            res[location] = base64Table[((uint8_t)(((uint32_t)(4227858432U) & group) >> 26))];
            res[location + 1] = base64Table[((uint8_t)(((uint32_t)(66060288U) & group) >> 20))];
            res[location + 2] = base64Table[((uint8_t)(((uint32_t)(1032192U) & group) >> 14))];
            res[location + 3] = base64Table[((uint8_t)(((uint32_t)(16128U) & group) >> 8))];
            group = 0;
        }

        if (extra == 1)
        {
            res[location] = base64Table[((uint8_t)(data[len] >> 2))];
            res[location + 1] = base64Table[((uint8_t)((data[len] & 3U) << 4))];
            res[location + 2] = '=';
            res[location + 3] = '=';
        }
        else if (extra == 2)
        {
            uint16_t t = (data[len] << 8) + (data[len + 1]);
            res[location] = base64Table[((uint8_t)((uint32_t)(t & 64512U) >> 10))];
            res[location + 1] = base64Table[((uint8_t)((uint32_t)(t & 1008U) >> 4))];
            res[location + 2] = base64Table[((uint8_t)((uint32_t)(t & 15U) << 2))];
            res[location + 3] = '=';
        }

        return res;
    }

    uint8_t Base64::getCharIndex(uint8_t c)
    {
        if (c > 96 && c < 123)
        {
            return c - 97 + 26;
        }
        else if (c > 64 && c < 91)
        {
            return c - 65;
        }
        else if (c > 47 && c < 58)
        {
            return c - 48 + 52;
        }
        else if (c == 43)
        {
            return 62;
        }
        else if (c == 47)
        {
            return 63;
        }
        else
        {
            return 0;
        }
    }

    uint8_t *Base64::base64Decode(uint8_t *data, uint64_t iLen, uint64_t *oLen)
    {
        *oLen = 0;

        if (!data)
        {
            return nullptr;
        }

        if (iLen % 4)
        {
            return nullptr;
        }

        uint64_t eIndex = 0;

        for (int i = 1; i < 3; i++)
        {
            if (data[iLen - i] == '=')
            {
                eIndex = i;
            }
        }

        iLen -= eIndex;

        uint8_t *output = new uint8_t[iLen / 4 * 3 + 3];
        // bzero(output, iLen / 4 * 3 + 2);

        uint8_t tLen = 0;
        uint8_t arr[4] = {0};

        for (uint64_t i = 0; i < iLen; i++)
        {
            arr[tLen++] = data[i];
            if (tLen >= 4)
            {
                output[(*oLen)++] = getCharIndex(arr[0]) << 2 | (getCharIndex(arr[1]) & 48U) >> 4;
                output[(*oLen)++] = getCharIndex(arr[1]) << 4 | (getCharIndex(arr[2]) & 60U) >> 2;
                output[(*oLen)++] = getCharIndex(arr[2]) << 6 | (getCharIndex(arr[3]) & 63U);
                tLen = 0;
            }
        }
        if (tLen == 2)
        {
            output[(*oLen)++] = (getCharIndex(arr[0])) << 2 | (getCharIndex(arr[1])) >> 4;
        }
        else if (tLen == 3)
        {
            output[(*oLen)++] = (getCharIndex(arr[0])) << 2 | (getCharIndex(arr[1])) >> 4;
            output[(*oLen)++] = (getCharIndex(arr[1])) << 4 | (getCharIndex(arr[2])) >> 2;
        }

        output[(*oLen)] = 0;

        return output;
    }

    // AES encryption and decryption according to ESPRESSIF ESP32 technical reference manual,
    // chapter 22, page 523(Chinese version)

#if CONFIG_IDF_TARGET_ESP32S3
    uint8_t *AES::aes256CBCEncrypt(
        const uint8_t *key,   // 32 bytes
        const uint8_t *iv,    // 16 bytes
        const uint8_t *plain, // plain
        uint32_t length,      // length of plain
        uint32_t *outLen      // length of output
    )
    {

        esp_crypto_sha_aes_lock_acquire();

        // padding plain with pkcs7 padding
        uint8_t paddingData = 16 - (length % 16);
        uint32_t bufferLength = length + paddingData;

        // allocate buffer to hold original data after padding
        uint8_t *bufferPadding = new (std::nothrow) uint8_t[bufferLength];

        if (!bufferPadding)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "memory allocate failed");
            (*outLen) = 0;
            return nullptr;
        }

        // copy original data
        memcpy(bufferPadding, plain, length);

        // padding
        memset(bufferPadding + length, paddingData, paddingData);

        // allocate buffer for uint32 array
        uint32_t *buffer = (uint32_t *)bufferPadding;

        // length for uint32 array after padding and convertion
        bufferLength /= 4;

        uint32_t *key32 = (uint32_t *)key;
        uint32_t *iv32 = (uint32_t *)iv;

        // xor first block with iv
        for (uint8_t i = 0; i < 4; ++i)
        {
            buffer[i] ^= iv32[i];
        }

        // Typical AES
        DPORT_REG_WRITE(AES_DMA_ENABLE_REG, (uint32_t)0);

        // config aes mode
        DPORT_REG_WRITE(AES_MODE_REG, (uint32_t)2);

        // fill key into register
        for (uint8_t i = 0; i < 8; ++i)
        {
            DPORT_REG_WRITE(AES_KEY_BASE + (i * 4), key32[i]);
        }

        // start encrypting
        for (uint32_t i = 0; i < bufferLength; i += 4)
        {
            // fill plain data(after padding) into register
            DPORT_REG_WRITE(AES_TEXT_IN_BASE, buffer[i]);
            DPORT_REG_WRITE(AES_TEXT_IN_BASE + 4, buffer[i + 1]);
            DPORT_REG_WRITE(AES_TEXT_IN_BASE + 8, buffer[i + 2]);
            DPORT_REG_WRITE(AES_TEXT_IN_BASE + 12, buffer[i + 3]);

            // start encrypting
            DPORT_REG_WRITE(AES_TRIGGER_REG, (uint32_t)1);

            // wait idle register
            while (DPORT_REG_READ(AES_STATE_REG))
            {
            }

            // read cipher data from register
            buffer[i] = DPORT_REG_READ(AES_TEXT_OUT_BASE);
            buffer[i + 1] = DPORT_REG_READ(AES_TEXT_OUT_BASE + 4);
            buffer[i + 2] = DPORT_REG_READ(AES_TEXT_OUT_BASE + 8);
            buffer[i + 3] = DPORT_REG_READ(AES_TEXT_OUT_BASE + 12);

            // encryption finished
            if (i + 4 >= bufferLength)
            {
                break;
            }

            // xor next block with former cipher text(cbc)
            buffer[i + 4] ^= buffer[i];
            buffer[i + 5] ^= buffer[i + 1];
            buffer[i + 6] ^= buffer[i + 2];
            buffer[i + 7] ^= buffer[i + 3];
        }

        uint8_t *u8Buffer = (uint8_t *)buffer;

        // return result
        (*outLen) = bufferLength * 4;

        esp_crypto_sha_aes_lock_release();

        return u8Buffer;
    }

#elif CONFIG_IDF_TARGET_ESP32
    uint8_t *AES::aes256CBCEncrypt(
        const uint8_t *key,   // 32 bytes
        const uint8_t *iv,    // 16 bytes
        const uint8_t *plain, // plain
        uint32_t length,      // length of plain
        uint32_t *outLen      // length of output
    )
    {
        // declare 32bit key and iv
        uint32_t key32[8] = {0};

        // data block is 128 bits, so iv as same as data block length
        uint32_t iv32[4] = {0};

        // convert key and iv to uint32 array
        for (uint8_t i = 0, j = 0; i < 8; ++i, j += 4)
        {
            key32[i] =
                (key[j] << 24) + (key[j + 1] << 16) + (key[j + 2] << 8) + (key[j + 3]);
            if (i < 4)
            {
                iv32[i] =
                    (iv[j] << 24) + (iv[j + 1] << 16) + (iv[j + 2] << 8) + (iv[j + 3]);
            }
        }

        // padding plain with pkcs7 padding
        uint8_t paddingData = 16 - (length % 16);
        uint32_t bufferLength = length + paddingData;

        // allocate buffer to hold original data after padding
        uint8_t *bufferPadding = new (std::nothrow) uint8_t[bufferLength];

        if (!bufferPadding)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "memory allocate failed");
            (*outLen) = 0;
            return nullptr;
        }

        // copy original data
        memcpy(bufferPadding, plain, length);

        // padding
        memset(bufferPadding + length, paddingData, paddingData);

        // allocate buffer for uint32 array
        uint32_t *buffer = new (std::nothrow) uint32_t[bufferLength / 4];
        if (!buffer)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "memory allocate failed");
            delete bufferPadding;
            (*outLen) = 0;
            return nullptr;
        }

        // convert uint8 array to uint32 array
        for (uint32_t i = 0, k = 0; i < bufferLength; i += 4)
        {
            buffer[k++] =
                (bufferPadding[i] << 24) + (bufferPadding[i + 1] << 16) + (bufferPadding[i + 2] << 8) + (bufferPadding[i + 3]);
        }

        // length for uint32 array after padding and convertion
        bufferLength /= 4;

        // remove padding buffer
        delete bufferPadding;

        // xor first block with iv
        for (uint8_t i = 0; i < 4; ++i)
        {
            buffer[i] ^= iv32[i];
        }

        // config aes mode and endian
        DPORT_REG_WRITE(AES_MODE_REG, (uint32_t)2);
        DPORT_REG_WRITE(AES_ENDIAN, (uint32_t)42);

        // fill key into register
        for (uint8_t i = 0; i < 8; ++i)
        {
            DPORT_REG_WRITE(AES_KEY_BASE + (i * 4), key32[i]);
        }

        // start encrypting
        for (uint32_t i = 0; i < bufferLength; i += 4)
        {
            // fill plain data(after padding) into register
            DPORT_REG_WRITE(AES_TEXT_BASE, buffer[i]);
            DPORT_REG_WRITE(AES_TEXT_BASE + 4, buffer[i + 1]);
            DPORT_REG_WRITE(AES_TEXT_BASE + 8, buffer[i + 2]);
            DPORT_REG_WRITE(AES_TEXT_BASE + 12, buffer[i + 3]);

            // start encrypting
            DPORT_REG_WRITE(AES_START_REG, (uint32_t)1);

            // wait idle register
            while (!(DPORT_REG_READ(AES_IDLE_REG)))
            {
            }

            // read cipher data from register
            buffer[i] = DPORT_REG_READ(AES_TEXT_BASE);
            buffer[i + 1] = DPORT_REG_READ(AES_TEXT_BASE + 4);
            buffer[i + 2] = DPORT_REG_READ(AES_TEXT_BASE + 8);
            buffer[i + 3] = DPORT_REG_READ(AES_TEXT_BASE + 12);

            // encryption finished
            if (i + 4 >= bufferLength)
            {
                break;
            }

            // xor next block with former cipher text(cbc)
            buffer[i + 4] ^= buffer[i];
            buffer[i + 5] ^= buffer[i + 1];
            buffer[i + 6] ^= buffer[i + 2];
            buffer[i + 7] ^= buffer[i + 3];
        }

        // convert uint32 array into uint8 array
        // this is optional, here only for uniform output and input
        uint8_t *u8Buffer = new uint8_t[bufferLength * 4];
        for (uint32_t i = 0, j = 0; i < bufferLength; ++i, j += 4)
        {
            u8Buffer[j] = (buffer[i] & (uint32_t)(0xff000000U)) >> 24;
            u8Buffer[j + 1] = (buffer[i] & (uint32_t)(0x00ff0000U)) >> 16;
            u8Buffer[j + 2] = (buffer[i] & (uint32_t)(0x0000ff00U)) >> 8;
            u8Buffer[j + 3] = (buffer[i] & (uint32_t)(0x000000ffU));
        }

        // remove uint32 array
        delete buffer;

        // return result
        (*outLen) = bufferLength * 4;
        return u8Buffer;
    }

#endif

#if CONFIG_IDF_TARGET_ESP32S3
    uint8_t *AES::aes256CBCDecrypt(
        const uint8_t *key,    // key
        const uint8_t *iv,     // iv
        const uint8_t *cipher, // cipher data
        uint32_t length,       // length of cipher data
        uint32_t *outLen       // length of output
    )
    {

        // length of input must be an integer of multiple of 16
        if (length % 16)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "data is invalid");
            (*outLen) = 0;
            return nullptr;
        }

        // calcuate length of uint32 array
        uint32_t bufferLength = length / 4;

        // allocate buffer
        uint32_t *buffer = (uint32_t *)cipher;

        uint32_t *key32 = (uint32_t *)key;
        uint32_t *iv32 = (uint32_t *)iv;

        // fill key into register
        for (uint8_t i = 0; i < 8; ++i)
        {
            DPORT_REG_WRITE(AES_KEY_BASE + (i * 4), key32[i]);
        }

        // config aes mode and endian
        DPORT_REG_WRITE(AES_MODE_REG, (uint32_t)6);

        // start decrypting
        // from last block to previous block
        for (uint32_t i = bufferLength - 4;;)
        {
            // fill cipher data to registers
            DPORT_REG_WRITE(AES_TEXT_IN_BASE, buffer[i]);
            DPORT_REG_WRITE(AES_TEXT_IN_BASE + 4, buffer[i + 1]);
            DPORT_REG_WRITE(AES_TEXT_IN_BASE + 8, buffer[i + 2]);
            DPORT_REG_WRITE(AES_TEXT_IN_BASE + 12, buffer[i + 3]);

            // start decrypting current block
            DPORT_REG_WRITE(AES_TRIGGER_REG, (uint32_t)1);

            // wait idle register
            while (DPORT_REG_READ(AES_STATE_REG))
            {
            }

            // read plain data
            buffer[i] = DPORT_REG_READ(AES_TEXT_OUT_BASE);
            buffer[i + 1] = DPORT_REG_READ(AES_TEXT_OUT_BASE + 4);
            buffer[i + 2] = DPORT_REG_READ(AES_TEXT_OUT_BASE + 8);
            buffer[i + 3] = DPORT_REG_READ(AES_TEXT_OUT_BASE + 12);

            // got plain data of current block

            // the first block has been decrypted
            // ready to break;
            if (!i)
            {
                break;
            }

            // xor plain data(after decryption) of current block with former block cipher data
            if (i >= 4)
            {
                buffer[i] ^= buffer[i - 4];
                buffer[i + 1] ^= buffer[i - 3];
                buffer[i + 2] ^= buffer[i - 2];
                buffer[i + 3] ^= buffer[i - 1];
                // got original data of current block
            }
            i -= 4;
        }

        // xor first block with iv
        for (uint8_t i = 0; i < 4; ++i)
        {
            buffer[i] ^= iv32[i];
        }

        // read padding length
        uint8_t paddingLength = (buffer[bufferLength - 1] & (uint32_t)(0xffU));
        if (!paddingLength || paddingLength > 0x10)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "length of padding is invalid");
            (*outLen) = 0;
            // delete buffer;
            return nullptr;
        }

        // get length of real data
        uint32_t realLength = (bufferLength * 4) - paddingLength;

        // allocate buffer for real data
        uint8_t *outputBuffer = new (std::nothrow) uint8_t[realLength];

        if (!outputBuffer)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "allocate memory failed");
            (*outLen) = 0;
            // delete buffer;
            return nullptr;
        }

        memcpy(outputBuffer, (uint8_t *)buffer, realLength);

        // remove uint32 array
        // delete buffer;

        // return data
        (*outLen) = realLength;
        return outputBuffer;
    }

#elif CONFIG_IDF_TARGET_ESP32
    uint8_t *AES::aes256CBCDecrypt(
        const uint8_t *key,    // key
        const uint8_t *iv,     // iv
        const uint8_t *cipher, // cipher data
        uint32_t length,       // length of cipher data
        uint32_t *outLen       // length of output
    )
    {

        // length of input must be an integer of multiple of 16
        if (length % 16)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "data is invalid");
            (*outLen) = 0;
            return nullptr;
        }

        // declare 32bit key and iv for convertion
        uint32_t key32[8] = {0};
        uint32_t iv32[4] = {0};

        // convert key and iv to uint32 array
        for (uint8_t i = 0, j = 0; i < 8; ++i, j += 4)
        {
            key32[i] =
                (key[j] << 24) + (key[j + 1] << 16) + (key[j + 2] << 8) + (key[j + 3]);
            if (i < 4)
            {
                iv32[i] =
                    (iv[j] << 24) + (iv[j + 1] << 16) + (iv[j + 2] << 8) + (iv[j + 3]);
            }
        }

        // calcuate length of uint32 array
        uint32_t bufferLength = length / 4;

        // allocate buffer
        uint32_t *buffer = new (std::nothrow) uint32_t[bufferLength];

        if (!buffer)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "buffer allocate failed when aes decrypting");
            (*outLen) = 0;
            return nullptr;
        }

        // convert uint8 array to uint32 array
        for (uint32_t i = 0, j = 0; i < bufferLength; ++i, j += 4)
        {
            buffer[i] =
                (cipher[j] << 24) + (cipher[j + 1] << 16) + (cipher[j + 2] << 8) + (cipher[j + 3]);
        }

        // fill key into register
        for (uint8_t i = 0; i < 8; ++i)
        {
            DPORT_REG_WRITE(AES_KEY_BASE + (i * 4), key32[i]);
        }

        // config aes mode and endian
        DPORT_REG_WRITE(AES_MODE_REG, (uint32_t)6);
        DPORT_REG_WRITE(AES_ENDIAN, (uint32_t)42);

        // start decrypting
        // from last block to previous block
        for (uint32_t i = bufferLength - 4;;)
        {
            // fill cipher data to registers
            DPORT_REG_WRITE(AES_TEXT_BASE, buffer[i]);
            DPORT_REG_WRITE(AES_TEXT_BASE + 4, buffer[i + 1]);
            DPORT_REG_WRITE(AES_TEXT_BASE + 8, buffer[i + 2]);
            DPORT_REG_WRITE(AES_TEXT_BASE + 12, buffer[i + 3]);

            // start decrypting current block
            DPORT_REG_WRITE(AES_START_REG, (uint32_t)1);

            // wait idle register
            while (!(DPORT_REG_READ(AES_IDLE_REG)))
            {
            }

            // read plain data
            buffer[i] = DPORT_REG_READ(AES_TEXT_BASE);
            buffer[i + 1] = DPORT_REG_READ(AES_TEXT_BASE + 4);
            buffer[i + 2] = DPORT_REG_READ(AES_TEXT_BASE + 8);
            buffer[i + 3] = DPORT_REG_READ(AES_TEXT_BASE + 12);
            // got plain data of current block

            // the first block has been decrypted
            // ready to break;
            if (!i)
            {
                break;
            }

            // xor plain data(after decryption) of current block with former block cipher data
            if (i >= 4)
            {
                buffer[i] ^= buffer[i - 4];
                buffer[i + 1] ^= buffer[i - 3];
                buffer[i + 2] ^= buffer[i - 2];
                buffer[i + 3] ^= buffer[i - 1];
                // got original data of current block
            }
            i -= 4;
        }

        // xor first block with iv
        for (uint8_t i = 0; i < 4; ++i)
        {
            buffer[i] ^= iv32[i];
        }

        // read padding length
        uint8_t paddingLength = (buffer[bufferLength - 1] & (uint32_t)(0xffU));
        if (!paddingLength || paddingLength > 0x10)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "length of padding is invalid");
            (*outLen) = 0;
            delete buffer;
            return nullptr;
        }

        // get length of real data
        uint32_t realLength = (bufferLength * 4) - paddingLength;

        // allocate buffer for real data
        uint8_t *outputBuffer = new (std::nothrow) uint8_t[realLength];

        if (!outputBuffer)
        {
            ESP_LOGD(MY_CRYPTO_DEBUG_HEADER, "allocate memory failed");
            (*outLen) = 0;
            delete buffer;
            return nullptr;
        }

        // copy real data from uint32 array to uint8 array
        for (uint32_t i = 0, j = 0, k = 0, t = 0xff000000U; i < realLength; ++i)
        {
            outputBuffer[i] = (uint8_t)((buffer[j] & t) >> (24 - (k << 3)));
            ++k;
            t >>= 8;
            if (!(k % 4))
            {
                ++j;
                k = 0;
                t = 0xff000000U;
            }
        }

        // remove uint32 array
        delete buffer;

        // return data
        (*outLen) = realLength;
        return outputBuffer;
    }

#endif
    // get hex string of aes 256 cbc
    String AES::aes256CBCEncrypt(String key,  // key
                                 String iv,   // iv
                                 String plain // data
    )
    {
        if (!key.length() || !iv.length() || !plain.length())
        {
            return "invalid input data";
        }

        if (key.length() != 32)
        {
            return "invalid length of key";
        }

        if (iv.length() != 16)
        {
            return "invalid length of iv";
        }

        uint32_t outLen = 0;
        uint8_t *buffer = aes256CBCEncrypt((uint8_t *)key.c_str(),
                                           (uint8_t *)iv.c_str(),
                                           (uint8_t *)plain.c_str(),
                                           plain.length(),
                                           &outLen);
        if (!outLen || !buffer)
        {
            if (buffer)
            {
                delete buffer;
            }
            return "invalid input data";
        }

        String output = "";
        char t[3];
        t[2] = 0;

        for (uint32_t i = 0; i < outLen; ++i)
        {
            sprintf(t, "%02x", buffer[i]);
            output += t;
        }

        delete buffer;

        return output;
    }

    // get original data of aes 256 cbc hex string
    String AES::aes256CBCDecrypt(
        String key,   // key
        String iv,    // iv
        String cipher // data, hex format
    )
    {
        if (!key.length() || !iv.length() || !cipher.length())
        {
            return "invalid input data";
        }

        if (cipher.length() % 16)
        {
            return "invalid length of data";
        }

        if (key.length() != 32)
        {
            return "invalid length of key";
        }

        if (iv.length() != 16)
        {
            return "invalid length of iv";
        }

        uint32_t bufferLength = cipher.length() / 2;

        uint8_t encryptedBuffer[bufferLength] = {0};

        if (!encryptedBuffer)
        {
            return "allocate memory failed";
        }

        for (uint32_t i = 0, j = 0; j < bufferLength; i += 2)
        {
            sscanf(cipher.substring(i, i + 2).c_str(), "%02x", (encryptedBuffer + j));
            ++j;
        }

        uint32_t outLen = 0;
        uint8_t *decryptedBuffer = aes256CBCDecrypt(
            (uint8_t *)key.c_str(),
            (uint8_t *)iv.c_str(),
            encryptedBuffer,
            bufferLength,
            &outLen);

        if (!outLen || !decryptedBuffer)
        {
            return "error when decrypting data";
        }

        String output = String((const char *)decryptedBuffer, outLen);
        delete decryptedBuffer;

        return output;
    }
}