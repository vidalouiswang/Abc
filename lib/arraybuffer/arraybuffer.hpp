/**
 * @file arraybuffer.hpp
 * @author Vida Wang (support@vida.wang)
 * @brief Two class in this file implement a simple method to pack a set of data with different types.
    The static function createArrayBuffer will convert a vector contains data into a binary uint8 array,
    which is could be sending through websocket or store it in flash.

    The static function decodeArrayBuffer will extract a list of data with different types from a single
    binary uint8 array, which could be using in everywhere.

    The class Element could store all integer types, float and double, string and uint8 array
    will be stored in free store in cpp.

    Two class will help beginners to handle binary data by a more easily way.


    这个文件里的两个类用一个简单的方法帮助新手去处理二进制数据，可以通过相关功能对不同类型的数据进行打包为一个二进制数组。
    静态函数createArrayBuffer可以把一个vector容器转换为一个独立的二进制数组，这个数据就可以用来在网络上传输或者存储了。

    静态函数decodeArrayBuffer可以从一个独立的二进制数组中提取出若干个不同类型的数据，这样可以方便在代码中使用。

    Element这个类用来存储一种数据，它可以是单字节、双字节、四字节、八字节的整数，单精度浮点、双精度浮点，字符串和另一个二进制数组。
    字符串和二进制数组使用同样的方式存储在C++的自由存储区中。
 * @version 1.0
 * @date 2022-07-22
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef ARRAY_BUFFER_H_
#define ARRAY_BUFFER_H_

#include <vector>
#include <Arduino.h>
#include <functional>
#include <math.h>
#include <mycrypto.h>

#define ARRAY_BUFFER_DEBUG_HEADER "Array Buffer"

// fuzzy comparison, will compare all types of integer, float and double if defined, operator ==
// 如果定义了此宏，将会对所有数字(整数、浮点数)进行对比，见重载运算符==
#define ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON

// will change data type automatically when doing math operations if defined
// for example, uint8_t data 0 - 1 will change to int8_t automatically
// attention: this function are in development
// 如果定义了此宏，在数学运算时会自动进行类型扩展
// 例如无符号8位数据 0 在 - 1时会自动修改为有符号8位
// 注意：此功能正在开发
#define AUTO_EXTEND_DATA_RANGE

// will handle "+", "-", "*" for different types if defined
// 如果定义了此宏，会处理不同类型之间的加减乘
#define CROSS_TYPE_CLACULATE

#define ARRAY_BUFFER_DEBUG_ON

// default constructor of "int" will be use auto set type if defined
// 如果定义了此宏，默认的 "int" 构造函数将会自动设置类型和数据
#define ENABLE_INT_DEFAULT_CONSTRUCTOR_AUTO_SET_TYPE_AND_DATA

typedef enum : uint8_t
{
    MARK_UINT8 = 0x80,
    MARK_UINT16 = 0x81,
    MARK_UINT32 = 0x82,
    MARK_UINT64 = 0x83,
    MARK_STRING = 0x86,
    MARK_BUFFER = 0x85,
    MARK_INT8 = 0x88,
    MARK_INT16 = 0x89,
    MARK_INT32 = 0x90,
    MARK_INT64 = 0x91,
    MARK_FLOAT = 0x92,
    MARK_DOUBLE = 0x93,
    MARK_EXTRA = 0x87
} EncodedBufferMark;

/**
 * @brief all you need right here
 * 你需要的一切类型都有了
 *
 */
typedef enum : int8_t
{
    // this type means current object storedd nothing
    // 这个类型表示当前对象存了个寂寞
    ETYPE_VOID = 0,

    ETYPE_UINT8 = 1,
    ETYPE_INT8 = -1,
    ETYPE_UINT16 = 2,
    ETYPE_INT16 = -2,
    ETYPE_UINT32 = 4,
    ETYPE_INT32 = -4,
    ETYPE_UINT64 = 8,
    ETYPE_INT64 = -8,

    ETYPE_FLOAT = 6,
    ETYPE_DOUBLE = 7,

    // indicate this element object hold a integer
    // 表示当前对象存储了一个整数
    ETYPE_INTEGER = 13,

    // you will got this type when you only want to know if there is a number
    // 当你只想知道是否是一个数字而不关心是几个字节的数字的时候就会返回这个类型
    ETYPE_NUMBER = 15,

    // string and uint8 array will use same way to store
    // 字符串和二进制数组使用同样的方式存储
    ETYPE_STRING = 9,
    ETYPE_BUFFER = 10,

    // reserved
    // 保留类型
    ETYPE_EXTRA = 11

} ElementType;

/**
 * @brief error code description
 * 错误代码描述
 *
 */
typedef enum : int8_t
{
    E_ERROR_NO_ERROR = 0,
    E_ERROR_HEAP_FULL = 1
} ElementErrorCode;

/**
 * @brief for class Element hold data in RAM
 * Element 类用来在内存中保存数据
 *
 */
typedef union
{
    uint8_t u8;
    int8_t i8;
    struct
    {
        uint8_t *p;            // 4 bytes
        uint32_t bufferLength; // 4 bytes
    } buffer;
    uint16_t u16;
    int16_t i16;
    uint32_t u32;
    int32_t i32;
    float f;
    uint64_t u64 = 0;
    int64_t i64;
    double d;
    struct
    {
        uint32_t msb;
        uint32_t lsb;
    } e;
} ElementData;

/**
 * @brief this class hold basic data in RAM
 * 这个类用来在RAM中存储基础数据
 */
class Element
{
private:
    /**
     * @brief data type of current object stored
     * 指示当前对象存储的数据类型
     */
    ElementType type = ETYPE_VOID;

    /**
     * @brief last error code of this element object
     * 这个对象的最后一个错误的错误代码
     *
     */
    ElementErrorCode err = E_ERROR_NO_ERROR;

    /**
     * @brief almost all data will be make a copy in this class
     * but in some cases, you wouldn't like to copy it
     * so there is a constructor with bool ahead will NOT copy original data
     * and this varible will indicate clear buffer or not when object deconstructing
     * default set to true
     *
     * 在这个类中，几乎所有的数据都会拷贝一份来存储
     * 但是有些时候你不想在创建一份拷贝
     * 所以有一个构造函数bool参数开头的可以只拷贝指针，而【不拷贝】原始数据
     * 这个变量用于指示是否需要清理拷贝的buffer
     * 默认是true
     */
    volatile bool copiedBuffer = true;

    /**
     * @brief hold data
     * 存储数据
     */
    ElementData data;

    /**
     * @brief this function could use same object to change its original data type to another
     * it will clean all buffer(if exists)
     * 这个函数用于给当前对象赋值另一种类型的数据
     * 如果当前对象存储了字符串或者拷贝模式的二进制数组
     * 那么就会清理掉缓存
     *
     * @param type new type 新的类型
     */
    void reset(ElementType type)
    {
        sizeof(Element);
        this->clearBuffer();

        // set current type to given type
        // 设定当前类型为给定的类型
        this->type = type;
    }

    /**
     * @brief set string buffer
     * 设置字符串
     *
     * @param data c string c字符串
     * @param offset offset that copy data from original buffer 从源数组开始拷贝的数据的偏移量
     * @param length length of c string 字符串的长度
     * @return true valid data 有效数据
     * @return false invalid data 无效数据
     */
    bool _setString(const char *data, uint32_t offset = 0, uint32_t length = 0)
    {
        if (data)
        {
            // get length of string
            // 获取目标字符串的长度
            if (!length)
            {
                length = strlen(data);
            }
            if (length)
            {
                // clear buffer if current object already stored data
                // 如果当前对象已经存储了其他数据会清除掉其他数据
                this->reset(ETYPE_STRING);

                // copy original data if length is non zero
                // 如果不是空字符串就拷贝数据
                return this->_copyBuffer((uint8_t *)data, length, offset, ETYPE_STRING);
            }
        }
        ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "type set to NONE");
        this->type = ETYPE_VOID;
        return false;
    }

    bool _setString(const String &data)
    {
        return this->_setString(data.c_str());
    }

    /**
     * @brief copy raw data from another area
     * 从另一个buffer拷贝原始数据
     *
     * @param buffer binary uint8 array 二进制数组指针
     * @param length length of data 数组长度
     * @param offset offset in source 需要从原始缓冲区开始拷贝的偏移量
     * @param type default type 默认类型
     *
     * @return true success 成功
     * @return false failed 失败
     */
    bool _copyBuffer(uint8_t *buffer, uint32_t length, uint32_t offset = 0, ElementType type = ETYPE_BUFFER)
    {
        // allocate buffer
        // 分配内存
        length = type == ETYPE_STRING ? length + 1 : length;
        this->data.buffer.p = new (std::nothrow) uint8_t[length];

        // check memory allocation
        // 检测内存分配
        if (!this->data.buffer.p)
        {
            this->type = ETYPE_VOID;
            this->err = E_ERROR_HEAP_FULL;
            this->data.buffer.bufferLength = 0;
            return false;
        }
        memset(this->data.buffer.p, 0, length);

        // copy from origin
        // 从原始位置拷贝数据
        memcpy(this->data.buffer.p, buffer + offset, length);
        if (type == ETYPE_STRING)
        {
            (this->data.buffer.p)[length - 1] = 0;
        }
        this->type = type;
        this->data.buffer.bufferLength = length;
        this->copiedBuffer = true;
        return true;
    }

    template <class T0, class T1>
    inline bool issame()
    {
        return std::is_same<T0, T1>::value;
    }

public:
    /**
     * @brief default constructor, type set to ETYPE_VOID
     * 默认无参构造函数, 类型被设置为ETYPE_VOID
     */
    inline Element() { this->type = ETYPE_VOID; }

    /**
     * @brief copy from another element, this is a special copy constructor
     * using pointer of another object
     * 拷贝构造函数， 可以从另一个对象拷贝一模一样的数据
     * @param e pointer of another object 另一个对象的指针
     */
    inline Element(const Element *e)
    {
        this->copyFrom((Element *)e);
    }

    inline Element(const Element &e)
    {
        this->copyFrom((Element *)(&e));
    }

    /**
     * @brief specific constructor
     * 专门类型的构造函数
     *
     * @param n number 数字
     */
    inline Element(const uint8_t &n) : type(ETYPE_UINT8) { this->data.u8 = n; }
    inline Element(const int8_t &n) : type(ETYPE_INT8) { this->data.i8 = n; };
    inline Element(const uint16_t &n) : type(ETYPE_UINT16) { this->data.u16 = n; };
    inline Element(const int16_t &n) : type(ETYPE_INT16) { this->data.i16 = n; };
    inline Element(const uint32_t &n) : type(ETYPE_UINT32) { this->data.u32 = n; };

#ifdef ENABLE_INT_DEFAULT_CONSTRUCTOR_AUTO_SET_TYPE_AND_DATA
    inline Element(const int32_t &n)
    {
        this->setNumber(n);
    }
#else
    inline Element(const int32_t &n) : type(ETYPE_INT32)
    {
        this->data.i32 = n;
    };
#endif

    inline Element(const uint64_t &n) : type(ETYPE_UINT64)
    {
        this->data.u64 = n;
    };
    inline Element(const int64_t &n) : type(ETYPE_INT64) { this->data.i64 = n; };
    inline Element(const float &n) : type(ETYPE_FLOAT) { this->data.f = n; };
    inline Element(const double &n) : type(ETYPE_DOUBLE) { this->data.d = n; };

    /**
     * @brief copy data from c string
     * 从c字符串中拷贝原始数据
     *
     * @param str c string; c字符串
     */
    inline Element(const char *str) : type(ETYPE_STRING) { this->_setString(str); }

    /**
     * @brief string constructor, always make a copy
     * 字符串的构造函数，会拷贝原始字符串
     *
     * @param str c string or arduino String(object or pointer) c字符串或Arduino字符串(对象或指针)
     */
    inline Element(const char *str, uint32_t offset, uint32_t length)
    {
        this->_setString(str, offset, length ? length : strlen(str));
    }
    inline Element(const String &str, uint32_t offset = 0)
    {
        this->_setString(str.c_str(), offset, str.length());
    }
    inline Element(const String *str, uint32_t offset = 0)
    {
        this->_setString(str->c_str(), offset, str->length());
    }
    inline Element(const std::string &str, uint32_t offset = 0)
    {
        this->_setString(str.c_str(), offset, str.length());
    }
    inline Element(const std::string *str, uint32_t offset = 0)
    {
        this->_setString(str->c_str(), offset, str->length());
    }

    /**
     * @brief raw uint8 array constructor, this will copy from original buffer
     * 二进制数组的构造函数
     *
     * @param buffer source uint8 array 源数组
     * @param bufferLength length of uint8 array 数组长度
     * @param copyOrigin copy origin data or not 是否拷贝原始数据
     * @param offsetInSrc offset in source 源数组开始拷贝的偏移量
     */
    explicit inline Element(uint8_t *buffer, uint32_t bufferLength, bool copyOrigin = true, uint32_t offsetInSrc = 0)
    {
        if (copyOrigin)
        {
            if (!this->_copyBuffer(buffer, bufferLength, offsetInSrc))
            {
                this->type = ETYPE_VOID;
                // this->data.buffer.p = nullptr;
                bzero(&(this->data), sizeof(ElementData));
                this->data.buffer.bufferLength = 0;
            }
        }
        else
        {
            this->reset(ETYPE_BUFFER);
            this->data.buffer.p = buffer;
            this->copiedBuffer = false;
        }
    }

#ifdef ARRAY_BUFFER_DEBUG_ON
    uint8_t *getRAM(int32_t *outLen = nullptr) const
    {
        if (outLen)
        {
            (*outLen) = sizeof(ElementData);
        }
        return (uint8_t *)(this->data.u64);
    }
#endif
    /**
     * @brief set type automatically by data range
     * 自动根据数据大小设置类型
     *
     * @note only integer match this function
     * 仅整数适用此函数
     *
     * this function fit in default int type to set correct type automatically
     * 这个函数适用于默认int自动设置对应的类型
     *
     * @param n number 数字
     * @param forceType manually set type 手动设置类型
     */
    void setNumber(int64_t n, ElementType forceType = ETYPE_VOID)
    {
        /**
         * @brief
         * in this part, it may should use like std::is_same<T, int>::value to set type and value
         * but in some cases, that may cause problem, so use number to set type and value
         *
         * 这部分可能应该使用类似于std::is_same<T, int>::value 来设置类型和数据
         * 但是在某些情况下可能会导致错误，所以直接使用数组的大小来设置类型和值
         *
         */
        this->clearBuffer();

        if (n >= 0)
        {
            // unsigned
            if (n < 256)
            {
                this->reset(ETYPE_UINT8);
                this->data.u8 = n;
            }
            else if (n > 0xff && n < 65536)
            {
                this->reset(ETYPE_UINT16);
                this->data.u16 = n;
            }
            else if (n > 0xffff && n < 4294967296)
            {
                this->reset(ETYPE_UINT32);
                this->data.u32 = n;
            }
            else
            {
                this->reset(ETYPE_UINT64);
                this->data.u64 = n;
            }
        }
        else
        {
            // signed
            if (n >= -128)
            {
                this->reset(ETYPE_INT8);
                this->data.i8 = n;
            }
            else if (n < -128 && n >= -32768)
            {
                this->reset(ETYPE_INT16);
                this->data.i16 = n;
            }
            else if (n < -32768 && n >= -2147483647)
            {
                this->reset(ETYPE_INT32);
                this->data.i32 = n;
            }
            else
            {
                this->reset(ETYPE_INT64);
                this->data.i64 = n;
            }
        }
        if (forceType != ETYPE_VOID)
        {
            this->type = forceType;
        }
    }

    /**
     * @brief this function will indicate current object if has available data
     * 这个函数用来指示当前对象是否存储了有用的数据
     *
     * @return true object stored valid data 对象存储了有效的数据可供读取
     * @return false object hasn't any valid data stored 对象没有存储任何有效数据
     */
    inline bool available() const
    {
        return this->type != ETYPE_VOID && this->err == E_ERROR_NO_ERROR;
    }

    /**
     * @brief manually copy object by developer
     * 开发者可以使用这两个函数手动从另一个对象或者缓存拷贝数据
     *
     * @param e pointer of another object
     * 另一个对象的指针
     * @return true success 成功
     * @return false failed 失败
     */
    bool copyFrom(Element *e)
    {
        if (!e)
        {
            ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "copy target is nullptr");
            return false;
        }

        if (e->getType() == ETYPE_VOID)
        {
            ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "the type of copy target has type ETYPE_VOID");
            return false;
        }

        this->clearBuffer();
        ElementType type = e->getType();
        this->type = type;
        switch (type)
        {
        case ETYPE_UINT8:
        case ETYPE_INT8:
            this->data.i8 = e->getInt8();
            break;
        case ETYPE_UINT16:
        case ETYPE_INT16:
            this->data.i16 = e->getInt16();
            break;
        case ETYPE_UINT32:
        case ETYPE_INT32:
            this->data.i32 = e->getInt32();
            break;
        case ETYPE_UINT64:
        case ETYPE_INT64:
            this->data.i64 = e->getInt64();
            break;
        case ETYPE_FLOAT:
            this->data.f = e->getFloat();
            break;
        case ETYPE_DOUBLE:
            this->data.d = e->getDouble();
            break;
        case ETYPE_STRING:
            return this->_setString((const char *)e->getRawBuffer(), 0, e->getRawBufferLength() - 1);
        case ETYPE_BUFFER:
            ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "copy from another element u8a");
            return this->copyFrom(e->getUint8Array(), e->getU8aLen());
        default:
            this->type = ETYPE_VOID;
            return false;
        }
        ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "copy finished");
        return true;
    }

    /**
     * @brief manually copy object by developer
     * 开发者可以使用这两个函数手动从另一个对象或者缓存拷贝数据
     *
     * @param element another object
     * 另一个对象
     * @return true success 成功
     * @return false failed 失败
     */
    inline bool copyFrom(Element element)
    {
        return this->copyFrom(&element);
    }

    /**
     * @brief copy data from uint8 array
     * 从一个buffer拷贝数据
     *
     * @param buffer pointer of source buffer 源数组指针
     * @param length length of data that will be copied 需要拷贝的长度
     * @return true success 成功
     * @return false failed 失败
     */
    bool copyFrom(uint8_t *buffer, uint64_t length)
    {
        this->clearBuffer();
        return this->_copyBuffer(buffer, length, 0);
    }

    /**
     * @brief check current object equals to another or not
     * 检查当前对象是否等于另一个对象
     *
     * @param obj another object; 另一个对象
     * @return true
     * @return false
     */
    bool equalsTo(const Element *obj) const
    {
        auto typeA = this->type, typeB = obj->getType();

        if (typeA != typeB)
        {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
            typeA = this->getType(true);
            typeB = obj->getType(true);

            double a = 0.0, b = 0.0;
            bool aFloat = false, bFloat = false;

            if (typeA == ETYPE_FLOAT)
            {
                a = this->data.f;
                aFloat = true;
            }
            else if (typeA == ETYPE_DOUBLE)
            {
                a = this->data.d;
                aFloat = true;
            }

            if (typeB == ETYPE_FLOAT)
            {
                b = obj->getFloat();
                bFloat = true;
            }
            else if (typeB == ETYPE_DOUBLE)
            {
                b = obj->getDouble();
                bFloat = true;
            }

            if (aFloat && bFloat)
            {
                // both float or double
                return a == b;
            }
            else if (aFloat && !bFloat)
            {
                // a is float or double, b is integer
                return a == obj->getNumber();
            }
            else if (!aFloat && bFloat)
            {
                // a is integer, b is float or double
                return this->getNumber() == b;
            }
            else
            {
                // both are integers
                return this->getNumber() == obj->getNumber();
            }

#else
            // return false if two elements have different type
            // 如果两个对象所存储的数据类型不一样直接返回false
            ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "type unequaled, a type: %d, b type: %d", obj->getType(), this->type);
            return false;
#endif
        }
        else
        {
            if (this->type == ETYPE_BUFFER || this->type == ETYPE_STRING)
            {
                // return false if length is unequaled of two elements when they stored uint8 array
                // 如果两个对象都存储了二进制数组但是长度不一样直接返回false
                // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "type: %d, lengthA: %u, lengthB: %u", obj->getBufferLength(), this->data.buffer.bufferLength);
                if (obj->getRawBufferLength() != this->data.buffer.bufferLength)
                {
                    // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "length not equal, length obj: %d, length this: %d\n string obj: [%s]\n string this: [%s]\n, ",
                    //          obj->getBufferLength(),
                    //          this->data.buffer.bufferLength,
                    //          obj->c_str(),
                    //          this->c_str());
                    return false;
                }
                else
                {
                    // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "string result: %s\n", h ? "true" : "false");
                    return !strncmp((const char *)(obj->getRawBuffer()), (const char *)(this->getRawBuffer()), this->data.buffer.bufferLength);
                }
            }
            else
            {
                switch (this->type)
                {
                    // compare number if they both stored numner
                    // 数字直接进行比较
                case ETYPE_UINT8:
                    return obj->getUint8() == this->data.u8;
                case ETYPE_INT8:
                    return obj->getInt8() == this->data.i8;
                case ETYPE_UINT16:
                    return obj->getUint16() == this->data.u16;
                case ETYPE_INT16:
                    return obj->getInt16() == this->data.i16;
                case ETYPE_UINT32:
                    return obj->getUint32() == this->data.u32;
                case ETYPE_INT32:
                    return obj->getInt32() == this->data.i32;
                case ETYPE_UINT64:
                    return obj->getUint64() == this->data.u64;
                case ETYPE_INT64:
                    return obj->getInt64() == this->data.i64;
                case ETYPE_FLOAT:
                    return obj->getFloat() == this->data.f;
                case ETYPE_DOUBLE:
                    return obj->getDouble() == this->data.d;
                default:
                    return false;
                }
            }
        }
        return false;
    }

    /**
     * @brief compare two elements
     * 比较两个对象
     *
     * @param b pointer to another object; 另一个对象的指针
     * @param lessThanB less than or greater than; 小于还是大于
     * @return true
     * @return false
     */
    bool compareElements(const Element *b, bool lessThanB) const
    {
        auto typeA = this->type, typeB = b->getType();

        if (typeA < 9)
        {
            // current object stored number
            if (typeB < 9)
            {
                // target object stored number
                if (lessThanB)
                    return this->getUniversalDouble() < b->getUniversalDouble();
                else
                    return this->getUniversalDouble() > b->getUniversalDouble();
            }
            else if (typeB == ETYPE_STRING || typeB == ETYPE_BUFFER)
            {
                // a is number, b is string or buffer
                // compare first byte

                if (lessThanB)
                    return this->getUniversalDouble() < b->getRawBuffer()[0];
                else
                    return this->getUniversalDouble() > b->getRawBuffer()[0];
            }
            else
            {
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "INVALID TYPE");
                return false;
            }
        }
        else
        {
            // a NOT a number
            if (typeA == ETYPE_BUFFER || typeA == ETYPE_STRING)
            {
                if (typeB < 9)
                {
                    // b is a number
                    if (lessThanB)
                        return (this->data.buffer.p)[0] < b->getUniversalDouble();
                    else
                        return (this->data.buffer.p)[0] > b->getUniversalDouble();
                }
                else
                {
                    // b NOT a number
                    if (typeB == ETYPE_BUFFER || typeB == ETYPE_STRING)
                    {
                        if (lessThanB)
                            return (this->data.buffer.p)[0] < b->getRawBuffer()[0];
                        else
                            return (this->data.buffer.p)[0] > b->getRawBuffer()[0];
                    }
                }
            }
            else
            {
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "INVALID TYPE");
                return false;
            }
        }
        return false;
    }

    /**
     * @brief convert any types of number to double and return;
     * it won't change object itself
     *
     * 转换所有类型的数字到双精度浮点，然后返回；
     * 这不会修改对象自身
     *
     * @return double
     */
    inline double getUniversalDouble() const
    {
        double a = 0.0;
        if (this->type == ETYPE_FLOAT)
        {
            a = this->data.f;
        }
        else if (this->type == ETYPE_DOUBLE)
        {
            a = this->data.d;
        }
        else
        {
            switch (this->type)
            {
            case ETYPE_UINT8:
                a = this->data.u8;
                break;
            case ETYPE_INT8:
                a = this->data.i8;
                break;
            case ETYPE_UINT16:
                a = this->data.u16;
                break;
            case ETYPE_INT16:
                a = this->data.i16;
                break;
            case ETYPE_UINT32:
                a = this->data.u32;
                break;
            case ETYPE_INT32:
                a = this->data.i32;
                break;
            case ETYPE_UINT64:
                a = this->data.u64;
                break;
            case ETYPE_INT64:
                a = this->data.i64;
                break;
            default:
                break;
            }
        }
        return a;
    }

    // operators, almost all operators were overloaded
    // 重载的运算符，几乎所有的运算符都已经被重载

    // =
    uint8_t operator=(const uint8_t &n)
    {
        this->reset(ETYPE_UINT8);
        this->data.u8 = n;
        return n;
    }

    int8_t operator=(const int8_t &n)
    {
        this->reset(ETYPE_INT8);
        this->data.i8 = n;
        return n;
    }

    uint16_t operator=(const uint16_t &n)
    {
        this->reset(ETYPE_UINT16);
        this->data.u16 = n;
        return n;
    }

    int16_t operator=(const int16_t &n)
    {
        this->reset(ETYPE_INT16);
        this->data.i16 = n;
        return n;
    }

    uint32_t operator=(const uint32_t &n)
    {
        this->reset(ETYPE_UINT32);
        this->data.u32 = n;
        return n;
    }

    int32_t operator=(const int32_t &n)
    {
        this->reset(ETYPE_INT32);
        this->data.i32 = n;
        return n;
    }

    uint64_t operator=(const uint64_t &n)
    {
        this->reset(ETYPE_UINT64);
        this->data.u64 = n;
        return n;
    }

    int64_t operator=(const int64_t &n)
    {
        this->reset(ETYPE_INT64);
        this->data.i64 = n;
        return n;
    }

    float operator=(const float &n)
    {
        this->reset(ETYPE_FLOAT);
        this->data.f = n;
        return n;
    }

    double operator=(const double &n)
    {
        this->reset(ETYPE_DOUBLE);
        this->data.d = n;
        return n;
    }

    const char *operator=(const char *str)
    {
        this->_setString(str);
        return str;
    }

    String operator=(const String &str)
    {
        this->_setString(str.c_str());
        return str;
    }

    const String *operator=(const String *str)
    {
        this->_setString(str->c_str());
        return str;
    }

    std::string operator=(const std::string &str)
    {
        this->_setString(str.c_str());
        return str;
    }

    const std::string *operator=(const std::string *str)
    {
        this->_setString(str->c_str());
        return str;
    }

    Element &operator=(const Element &e)
    {
        this->copyFrom(e);
        return *this;
    }

    const Element *operator=(const Element *e)
    {
        this->copyFrom(*e);
        return e;
    }

    // ()

    /**
     * @brief set an uint8 array to element
     *
     * @param buffer pointer
     * @param length length of buffer
     * @return Element& self
     */
    Element &operator()(uint8_t *buffer, uint64_t length)
    {
        this->copyFrom(buffer, length);
        return *this;
    }

    // ==

    inline bool operator==(const char *str) const
    {
        uint32_t targetLen = strlen(str);
        if (!targetLen)
        {
            if (!this->available())
            {
                return true;
            }
            else if (this->type == ETYPE_STRING && (this->data.buffer.bufferLength - 1 <= 0))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        if (targetLen != (this->data.buffer.bufferLength - 1))
        {
            return false;
        }
        else
        {
            return !strncmp(str, this->c_str(), targetLen);
        }
    }

    inline bool operator==(const String &str) const
    {
        return this->operator==(str.c_str());
    }

    inline bool operator==(const String *str) const
    {
        return this->operator==(str->c_str());
    }

    inline bool operator==(const std::string &str) const
    {
        return this->operator==(str.c_str());
    }

    inline bool operator==(const std::string *str) const
    {
        return this->operator==(str->c_str());
    }

    inline bool operator==(const Element &obj) const
    {
        return obj.equalsTo(this);
    }

    inline bool operator==(const Element *obj) const
    {
        return obj->equalsTo(this);
    }

    inline bool operator==(const uint8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() == n;
#else
        if (this->type == ETYPE_UINT8)
        {
            return this->data.u8 == n;
        }
        return false;
#endif
    }

    inline bool operator==(const int8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() == n;
#else
        if (this->tpe == ETYPE_INT8)
        {
            return this->data.i8 == n;
        }
        return false;
#endif
    }

    inline bool operator==(const uint16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() == n;
#else
        if (this->type == ETYPE_UINT16)
        {
            return this->data.u16 == n;
        }
        return false;
#endif
    }

    inline bool operator==(const int16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() == n;
#else
        if (this->type == ETYPE_INT16)
        {
            return this->data.i16 == n;
        }
        return false;
#endif
    }

    inline bool operator==(const uint32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() == n;
#else
        if (this->type == ETYPE_UINT32)
        {
            return this->data.u32 == n;
        }
        return false;
#endif
    }

    inline bool operator==(const int32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() == n;
#else
        if (this->type == ETYPE_INT32)
        {
            return this->data.i32 == n;
        }
        return false;
#endif
    }

    inline bool operator==(const uint64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() == n;
#else
        if (this->type == ETYPE_UINT64)
        {
            return this->data.u64 == n;
        }
        return false;
#endif
    }

    inline bool operator==(const int64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() == n;
#else
        if (this->type == ETYPE_INT64)
        {
            return this->data.i64 == n;
        }
        return false;
#endif
    }

    inline bool operator==(const float &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() == n;
#else
        if (this->type == ETYPE_FLOAT)
        {
            return this->data.f == n;
        }
        return false;
#endif
    }

    inline bool operator==(const double &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->data.d == n;
#else
        if (this->type == ETYPE_DOUBLE)
        {
            return this->data.d == n;
        }
        return false;
#endif
    }

    // !=

    inline bool operator!=(const Element &obj) const
    {
        return !obj.equalsTo(this);
    }

    inline bool operator!=(const Element *obj) const
    {
        return !obj->equalsTo(this);
    }

    inline bool operator!=(const uint8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() != n;
#else
        if (this->type == ETYPE_UINT8)
        {
            return this->data.u8 != n;
        }
        return false;
#endif
    }

    inline bool operator!=(const int8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() != n;
#else
        if (this->tpe == ETYPE_INT8)
        {
            return this->data.i8 != n;
        }
        return false;
#endif
    }

    inline bool operator!=(const uint16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() != n;
#else
        if (this->type == ETYPE_UINT16)
        {
            return this->data.u16 != n;
        }
        return false;
#endif
    }

    inline bool operator!=(const int16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() != n;
#else
        if (this->type == ETYPE_INT16)
        {
            return this->data.i16 != n;
        }
        return false;
#endif
    }

    inline bool operator!=(const uint32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() != n;
#else
        if (this->type == ETYPE_UINT32)
        {
            return this->data.u32 != n;
        }
        return false;
#endif
    }

    inline bool operator!=(const int32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() != n;
#else
        if (this->type == ETYPE_INT32)
        {
            return this->data.i32 != n;
        }
        return false;
#endif
    }

    inline bool operator!=(const uint64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() != n;
#else
        if (this->type == ETYPE_UINT64)
        {
            return this->data.u64 != n;
        }
        return false;
#endif
    }

    inline bool operator!=(const int64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() != n;
#else
        if (this->type == ETYPE_INT64)
        {
            return this->data.i64 != n;
        }
        return false;
#endif
    }

    inline bool operator!=(const float &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() != n;
#else
        if (this->type == ETYPE_FLOAT)
        {
            return this->data.f != n;
        }
        return false;
#endif
    }

    inline bool operator!=(const double &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->data.d != n;
#else
        if (this->type == ETYPE_DOUBLE)
        {
            return this->data.d != n;
        }
        return false;
#endif
    }

    //<
    inline bool operator<(const uint8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_UINT8)
        {
            return this->data.u8 < n;
        }
        return false;
#endif
    }

    inline bool operator<(const int8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_INT8)
        {
            return this->data.i8 < n;
        }
        return false;
#endif
    }

    inline bool operator<(const uint16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_UINT16)
        {
            return this->data.u16 < n;
        }
        return false;
#endif
    }

    inline bool operator<(const int16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_INT16)
        {
            return this->data.i16 < n;
        }
        return false;
#endif
    }

    inline bool operator<(const uint32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_UINT32)
        {
            return this->data.u32 < n;
        }
        return false;
#endif
    }

    inline bool operator<(const int32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_INT32)
        {
            return this->data.i32 < n;
        }
        return false;
#endif
    }

    inline bool operator<(const uint64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_UINT64)
        {
            return this->data.u64 < n;
        }
        return false;
#endif
    }

    inline bool operator<(const int64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_INT64)
        {
            return this->data.i64 < n;
        }
        return false;
#endif
    }

    inline bool operator<(const float &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_FLOAT)
        {
            return this->data.f < n;
        }
        return false;
#endif
    }

    inline bool operator<(const double &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < n;
#else
        if (this->type == ETYPE_DOUBLE)
        {
            return this->data.d < n;
        }
        return false;
#endif
    }

    inline bool operator<(const char *str) const
    {
        if (!str || !strlen(str))
            return false;
#ifndef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() < str[0];
#else
        if (this->type == ETYPE_STRING)
        {
            int32_t sizeA = this->data.buffer.bufferLength - 1;
            int32_t sizeB = strlen(str);
            return strncmp(this->c_str(), str, sizeA < sizeB ? sizeA : sizeB) < 0;
        }
        return false;
#endif
    }

    inline bool operator<(const String &str) const
    {
        return this->operator<(str.c_str());
    }

    inline bool operator<(const String *str) const
    {
        return this->operator<(str->c_str());
    }

    inline bool operator<(const std::string &str) const
    {
        return this->operator<(str.c_str());
    }

    inline bool operator<(const std::string *str) const
    {
        return this->operator<(str->c_str());
    }

    inline bool operator<(const Element &e) const
    {
        return this->compareElements(&e, true);
    }

    inline bool operator<(const Element *e) const
    {
        return this->compareElements(e, true);
    }

    //>
    inline bool operator>(const uint8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_UINT8)
        {
            return this->data.u8 > n;
        }
        return false;
#endif
    }

    inline bool operator>(const int8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_INT8)
        {
            return this->data.i8 > n;
        }
        return false;
#endif
    }

    inline bool operator>(const uint16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_UINT16)
        {
            return this->data.u16 > n;
        }
        return false;
#endif
    }

    inline bool operator>(const int16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_INT16)
        {
            return this->data.i16 > n;
        }
        return false;
#endif
    }

    inline bool operator>(const uint32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_UINT32)
        {
            return this->data.u32 > n;
        }
        return false;
#endif
    }

    inline bool operator>(const int32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_INT32)
        {
            return this->data.i32 > n;
        }
        return false;
#endif
    }

    inline bool operator>(const uint64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_UINT64)
        {
            return this->data.u64 > n;
        }
        return false;
#endif
    }

    inline bool operator>(const int64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_INT64)
        {
            return this->data.i64 > n;
        }
        return false;
#endif
    }

    inline bool operator>(const float &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_FLOAT)
        {
            return this->data.f > n;
        }
        return false;
#endif
    }

    inline bool operator>(const double &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > n;
#else
        if (this->type == ETYPE_DOUBLE)
        {
            return this->data.d > n;
        }
        return false;
#endif
    }

    inline bool operator>(const char *str) const
    {
        if (!str || !strlen(str))
            return false;
#ifndef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() > str[0];
#else
        if (this->type == ETYPE_STRING)
        {
            int32_t sizeA = this->data.buffer.bufferLength - 1;
            int32_t sizeB = strlen(str);
            return strncmp(this->c_str(), str, sizeA > sizeB ? sizeA : sizeB) > 0;
        }
        return false;
#endif
    }

    inline bool operator>(const String &str) const
    {
        return this->operator>(str.c_str());
    }

    inline bool operator>(const String *str) const
    {
        return this->operator>(str->c_str());
    }

    inline bool operator>(const std::string &str) const
    {
        return this->operator>(str.c_str());
    }

    inline bool operator>(const std::string *str) const
    {
        return this->operator>(str->c_str());
    }

    inline bool operator>(const Element &e) const
    {
        return this->compareElements(&e, false);
    }

    inline bool operator>(const Element *e) const
    {
        return this->compareElements(e, false);
    }

    //<=

    inline bool operator<=(const uint8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_UINT8)
        {
            return this->data.u8 <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const int8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_INT8)
        {
            return this->data.i8 <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const uint16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_UINT16)
        {
            return this->data.u16 <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const int16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_INT16)
        {
            return this->data.i16 <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const uint32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_UINT32)
        {
            return this->data.u32 <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const int32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_INT32)
        {
            return this->data.i32 <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const uint64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_UINT64)
        {
            return this->data.u64 <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const int64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_INT64)
        {
            return this->data.i64 <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const float &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_FLOAT)
        {
            return this->data.f <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const double &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= n;
#else
        if (this->type == ETYPE_DOUBLE)
        {
            return this->data.d <= n;
        }
        return false;
#endif
    }

    inline bool operator<=(const char *str) const
    {
        if (!str || !strlen(str))
            return false;
#ifndef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() <= str[0];
#else
        if (this->type == ETYPE_STRING)
        {
            int32_t sizeA = this->data.buffer.bufferLength - 1;
            int32_t sizeB = strlen(str);
            return strncmp(this->c_str(), str, sizeA <= sizeB ? sizeA : sizeB) <= 0;
        }
        return false;
#endif
    }

    inline bool operator<=(const String &str) const
    {
        return this->operator<=(str.c_str());
    }

    inline bool operator<=(const String *str) const
    {
        return this->operator<=(str->c_str());
    }

    inline bool operator<=(const std::string &str) const
    {
        return this->operator<=(str.c_str());
    }

    inline bool operator<=(const std::string *str) const
    {
        return this->operator<=(str->c_str());
    }

    inline bool operator<=(const Element &e) const
    {
        return this->compareElements(&e, true) || (*this) == e;
    }

    inline bool operator<=(const Element *e) const
    {
        return this->compareElements(e, true) || (*this) == (*e);
    }

    //>=

    inline bool operator>=(const uint8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_UINT8)
        {
            return this->data.u8 >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const int8_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_INT8)
        {
            return this->data.i8 >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const uint16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_UINT16)
        {
            return this->data.u16 >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const int16_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_INT16)
        {
            return this->data.i16 >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const uint32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_UINT32)
        {
            return this->data.u32 >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const int32_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_INT32)
        {
            return this->data.i32 >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const uint64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_UINT64)
        {
            return this->data.u64 >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const int64_t &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_INT64)
        {
            return this->data.i64 >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const float &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_FLOAT)
        {
            return this->data.f >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const double &n) const
    {
#ifdef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= n;
#else
        if (this->type == ETYPE_DOUBLE)
        {
            return this->data.d >= n;
        }
        return false;
#endif
    }

    inline bool operator>=(const char *str) const
    {
        if (!str || !strlen(str))
            return false;
#ifndef ENABLE_ELEMENT_NUMBER_FUZZY_COMPARISON
        return this->getUniversalDouble() >= str[0];
#else
        if (this->type == ETYPE_STRING)
        {
            int32_t sizeA = this->data.buffer.bufferLength - 1;
            int32_t sizeB = strlen(str);
            return strncmp(this->c_str(), str, sizeA >= sizeB ? sizeA : sizeB) >= 0;
        }
        return false;
#endif
    }

    inline bool operator>=(const String &str) const
    {
        return this->operator>=(str.c_str());
    }

    inline bool operator>=(const String *str) const
    {
        return this->operator>=(str->c_str());
    }

    inline bool operator>=(const std::string &str) const
    {
        return this->operator>=(str.c_str());
    }

    inline bool operator>=(const std::string *str) const
    {
        return this->operator>=(str->c_str());
    }

    inline bool operator>=(const Element &e) const
    {
        return this->compareElements(&e, false) || (*this) == e;
    }

    inline bool operator>=(const Element *e) const
    {
        return this->compareElements(e, false) || (*this) == (*e);
    }

    // basic math operators
    // 基本数学符号重载

    // +

    uint8_t operator+(const uint8_t &n) const
    {
        return (uint8_t)(this->getUniversalDouble() + n);
    }
    int8_t operator+(const int8_t &n) const
    {
        return (int8_t)(this->getUniversalDouble() + n);
    }

    uint16_t operator+(const uint16_t &n) const
    {
        return (uint16_t)(this->getUniversalDouble() + n);
    }
    int16_t operator+(const int16_t &n) const
    {
        return (int16_t)(this->getUniversalDouble() + n);
    }

    uint32_t operator+(const uint32_t &n) const
    {
        return (uint32_t)(this->getUniversalDouble() + n);
    }
    int32_t operator+(const int32_t &n) const
    {
        return (int32_t)(this->getUniversalDouble() + n);
    }

    uint64_t operator+(const uint64_t &n) const
    {
        return (uint64_t)(this->getUniversalDouble() + n);
    }
    int64_t operator+(const int64_t &n) const
    {
        return (int64_t)(this->getUniversalDouble() + n);
    }

    // becarefull using this, it will lose precision
    // 小心使用这个函数，会丢失精度
    float operator+(const float &n) const
    {
        return (float)(this->getUniversalDouble() + n);
    }

    double operator+(const double &n) const
    {
        return (double)(this->getUniversalDouble() + n);
    }

    // Element + Element
    Element operator+(const Element &rvalue) const
    {
        Element tmp;
        auto typeB = rvalue.getType();

#ifdef CROSS_TYPE_CLACULATE
        if (this->type < 9)
        {
            if (typeB < 9) // both numbers
            {
                int8_t direction = this->type < 0 || typeB < 0 ? -1 : 1;
                ElementType newType = this->type > typeB ? this->type : typeB;
                newType = (ElementType)((int8_t)newType * direction);

                if (newType == 6 || newType == 7)
                {
                    tmp = this->getUniversalDouble() + rvalue.getUniversalDouble();
                }
                else
                {
                    tmp = this->getNumber() + rvalue.getNumber();
                }
            }
            else // b isn't number
            {
                if (typeB == ETYPE_STRING)
                {
                    // b will add to tail of the string

                    switch (this->type)
                    {
                    case ETYPE_UINT8:
                    case ETYPE_INT8:
                    case ETYPE_UINT16:
                    case ETYPE_INT16:
                    case ETYPE_UINT32:
                    case ETYPE_INT32:
                        tmp = String(this->data.i32) + rvalue.getString();
                        break;
                    case ETYPE_UINT64:
                    case ETYPE_INT64:
                    {
                        char buf[32] = {0};
                        sprintf(buf,
                                (this->type == ETYPE_UINT64 ? "%llu" : "%ld"),
                                (this->type == ETYPE_UINT64 ? this->data.u64 : this->data.i64));
                        tmp = String(buf) + rvalue.getString();
                    }
                    break;
                    case ETYPE_FLOAT:
                    case ETYPE_DOUBLE:
                        tmp = String(this->getUniversalDouble()) + rvalue.getString();
                        break;
                    case ETYPE_STRING:
                        tmp = this->getString() + rvalue.getString();
                        break;
                    }
                }
                // not support buffer add
            }
        }
        else // a isn't number
        {
            if (typeB < 9) // b is number, a isn't number
            {
                if (this->type == ETYPE_STRING)
                {
                    switch (typeB)
                    {
                    case ETYPE_UINT8:
                    case ETYPE_INT8:
                    case ETYPE_UINT16:
                    case ETYPE_INT16:
                    case ETYPE_UINT32:
                    case ETYPE_INT32:
                        tmp = this->getString() + String(rvalue.getInt32());
                        break;
                    case ETYPE_UINT64:
                    case ETYPE_INT64:
                    {
                        char buf[32] = {0};
                        sprintf(buf,
                                (typeB == ETYPE_UINT64 ? "%llu" : "%ld"),
                                (typeB == ETYPE_UINT64 ? rvalue.getUint64() : rvalue.getInt64()));
                        tmp = this->getString() + String(buf);
                    }
                    break;
                    case ETYPE_FLOAT:
                    case ETYPE_DOUBLE:
                        tmp = this->getString() + String(rvalue.getUniversalDouble());
                        break;
                    }

                    // not support buffer add
                }
            }
            else // b isn't number, a ins't number
            {
                if (typeB == ETYPE_STRING) // a is string, b is number
                {
                    // b will add to tail of the string

                    tmp = this->getString() + rvalue.getString();

                    // not support buffer add
                }
            }
        }

#else
        if (this->type == rvalue.getType())
        {
            if (this->type == ETYPE_STRING) // both strings
            {
                tmp = this->getString() + rvalue.getString();
            }
            if (this->type < 9 && typeB < 9)
            {
                switch (this->type)
                {
                case ETYPE_UINT8:
                    tmp = this->data.u8 + rvalue.getUint8();
                    break;
                case ETYPE_INT8:
                    tmp = this->data.i8 + rvalue.getInt8();
                    break;
                case ETYPE_UINT16:
                    tmp = this->data.u16 + rvalue.getUint16();
                    break;
                case ETYPE_INT16:
                    tmp = this->data.i16 + rvalue.getInt16();
                    break;
                case ETYPE_UINT32:
                    tmp = this->data.u32 + rvalue.getUint32();
                    break;
                case ETYPE_INT32:
                    tmp = this->data.i32 + rvalue.getInt32();
                    break;
                case ETYPE_UINT64:
                    tmp = this->data.u64 + rvalue.getUint64();
                    break;
                case ETYPE_INT64:
                    tmp = this->data.i64 + rvalue.getInt64();
                    break;
                case ETYPE_FLOAT:
                    tmp = this->data.f + rvalue.getFloat();
                    break;
                case ETYPE_DOUBLE:
                    tmp = this->data.d + rvalue.getDouble();
                    break;
                }
            }
        }
#endif
        return tmp;
    }

    //-

    uint8_t operator-(const uint8_t &n) const
    {
        return (uint8_t)(this->getUniversalDouble() - n);
    }
    int8_t operator-(const int8_t &n) const
    {
        return (int8_t)(this->getUniversalDouble() - n);
    }

    uint16_t operator-(const uint16_t &n) const
    {
        return (uint16_t)(this->getUniversalDouble() - n);
    }
    int16_t operator-(const int16_t &n) const
    {
        return (int16_t)(this->getUniversalDouble() - n);
    }

    uint32_t operator-(const uint32_t &n) const
    {
        return (uint32_t)(this->getUniversalDouble() - n);
    }
    int32_t operator-(const int32_t &n) const
    {
        return (int32_t)(this->getUniversalDouble() - n);
    }

    uint64_t operator-(const uint64_t &n) const
    {
        return (uint64_t)(this->getUniversalDouble() - n);
    }
    int64_t operator-(const int64_t &n) const
    {
        return (int64_t)(this->getUniversalDouble() - n);
    }

    // becarefull using this, it will lose precision
    // 小心使用这个函数，会丢失精度
    float operator-(const float &n) const
    {
        return (float)(this->getUniversalDouble() - n);
    }

    double operator-(const double &n) const
    {
        return (double)(this->getUniversalDouble() - n);
    }

    // Element - Element
    Element operator-(const Element &rvalue) const
    {
        Element tmp;
        auto typeB = rvalue.getType();

#ifdef CROSS_TYPE_CLACULATE
        if (this->type < 9)
        {
            if (typeB < 9) // both numbers
            {
                int8_t direction = this->type < 0 || typeB < 0 ? -1 : 1;
                ElementType newType = this->type > typeB ? this->type : typeB;
                newType = (ElementType)((int8_t)newType * direction);

                if (newType == 6 || newType == 7)
                {
                    tmp = this->getUniversalDouble() - rvalue.getUniversalDouble();
                }
                else
                {
                    tmp = this->getNumber() - rvalue.getNumber();
                }
            }
            else // b isn't number
            {
                // a short string replace a long string(maybe) is not useful
            }
        }
        else // a isn't number
        {
            if (typeB < 9) // b is number, a isn't number
            {
                if (this->type == ETYPE_STRING)
                {
                    tmp = this->getString();
                    switch (typeB)
                    {
                    case ETYPE_UINT8:
                    case ETYPE_INT8:
                    case ETYPE_UINT16:
                    case ETYPE_INT16:
                    case ETYPE_UINT32:
                    case ETYPE_INT32:
                        tmp.replace(String(rvalue.getInt32()), "");
                        break;
                    case ETYPE_UINT64:
                    case ETYPE_INT64:
                    {
                        char buf[32] = {0};
                        sprintf(buf,
                                (typeB == ETYPE_UINT64 ? "%llu" : "%ld"),
                                (typeB == ETYPE_UINT64 ? rvalue.getUint64() : rvalue.getInt64()));
                        tmp.replace(String(buf), "");
                    }
                    break;
                    case ETYPE_FLOAT:
                    case ETYPE_DOUBLE:
                        tmp.replace(String(String(rvalue.getUniversalDouble())), "");
                        break;
                    }

                    // not support buffer add
                }
            }
            else // b isn't number, a ins't number
            {
                if (typeB == ETYPE_STRING) // a is string, b is number
                {
                    // b will add to tail of the string

                    tmp = this->getString();
                    tmp.replace(rvalue.getString(), "");

                    // not support buffer add
                }
            }
        }

#else
        if (this->type == rvalue.getType())
        {
            if (this->type == ETYPE_STRING) // both strings
            {
                tmp = this->getString();
                tmp.replace(rvalue.getString(), "");
            }
            if (this->type < 9 && typeB < 9)
            {
                switch (this->type)
                {
                case ETYPE_UINT8:
                    tmp = this->data.u8 - rvalue.getUint8();
                    break;
                case ETYPE_INT8:
                    tmp = this->data.i8 - rvalue.getInt8();
                    break;
                case ETYPE_UINT16:
                    tmp = this->data.u16 - rvalue.getUint16();
                    break;
                case ETYPE_INT16:
                    tmp = this->data.i16 - rvalue.getInt16();
                    break;
                case ETYPE_UINT32:
                    tmp = this->data.u32 - rvalue.getUint32();
                    break;
                case ETYPE_INT32:
                    tmp = this->data.i32 - rvalue.getInt32();
                    break;
                case ETYPE_UINT64:
                    tmp = this->data.u64 - rvalue.getUint64();
                    break;
                case ETYPE_INT64:
                    tmp = this->data.i64 - rvalue.getInt64();
                    break;
                case ETYPE_FLOAT:
                    tmp = this->data.f - rvalue.getFloat();
                    break;
                case ETYPE_DOUBLE:
                    tmp = this->data.d - rvalue.getDouble();
                    break;
                }
            }
        }
#endif
        return tmp;
    }

    //*

    uint8_t operator*(const uint8_t &n) const
    {
        return (uint8_t)(this->getUniversalDouble() * n);
    }
    int8_t operator*(const int8_t &n) const
    {
        return (int8_t)(this->getUniversalDouble() * n);
    }

    uint16_t operator*(const uint16_t &n) const
    {
        return (uint16_t)(this->getUniversalDouble() * n);
    }
    int16_t operator*(const int16_t &n) const
    {
        return (int16_t)(this->getUniversalDouble() * n);
    }

    uint32_t operator*(const uint32_t &n) const
    {
        return (uint32_t)(this->getUniversalDouble() * n);
    }
    int32_t operator*(const int32_t &n) const
    {
        return (int32_t)(this->getUniversalDouble() * n);
    }

    uint64_t operator*(const uint64_t &n) const
    {
        return (uint64_t)(this->getUniversalDouble() * n);
    }
    int64_t operator*(const int64_t &n) const
    {
        return (int64_t)(this->getUniversalDouble() * n);
    }

    // becarefull using this, it will lose precision
    // 小心使用这个函数，会丢失精度
    float operator*(const float &n) const
    {
        return (float)(this->getUniversalDouble() * n);
    }

    double operator*(const double &n) const
    {
        return (double)(this->getUniversalDouble() * n);
    }

    // Element * Element
    Element operator*(const Element &rvalue) const
    {
        Element tmp;
        auto typeB = rvalue.getType();

#ifdef CROSS_TYPE_CLACULATE
        if (this->type < 9)
        {
            if (typeB < 9) // both numbers
            {
                int8_t direction = this->type < 0 || typeB < 0 ? -1 : 1;
                ElementType newType = this->type > typeB ? this->type : typeB;
                newType = (ElementType)((int8_t)newType * direction);

                if (newType == 6 || newType == 7)
                {
                    tmp = this->getUniversalDouble() * rvalue.getUniversalDouble();
                }
                else
                {
                    tmp = this->getNumber() * rvalue.getNumber();
                }
            }
            else // b isn't number, a is number
            {
                if (typeB == ETYPE_STRING)
                {
                    switch (this->type)
                    {
                    case ETYPE_UINT8:
                    case ETYPE_INT8:
                    case ETYPE_UINT16:
                    case ETYPE_INT16:
                    case ETYPE_UINT32:
                    case ETYPE_INT32:
                    case ETYPE_UINT64:
                    case ETYPE_INT64:
                    case ETYPE_FLOAT:
                    case ETYPE_DOUBLE:
                        tmp = rvalue.getString();
                        int64_t times = this->getNumber();
                        for (int i = 0; i < times; ++i)
                        {
                            tmp = tmp.getString() + tmp.getString();
                        }
                        break;
                    }
                }
            }
        }
        else // a isn't number
        {
            if (typeB < 9) // b is number, a isn't number
            {
                if (this->type == ETYPE_STRING)
                {
                    switch (typeB)
                    {
                    case ETYPE_UINT8:
                    case ETYPE_INT8:
                    case ETYPE_UINT16:
                    case ETYPE_INT16:
                    case ETYPE_UINT32:
                    case ETYPE_INT32:
                    case ETYPE_UINT64:
                    case ETYPE_INT64:
                    case ETYPE_FLOAT:
                    case ETYPE_DOUBLE:
                        tmp = this->getString();
                        int64_t times = rvalue.getNumber();
                        String tmpStr = this->getString();
                        for (int i = 0; i < times - 1; ++i)
                        {
                            tmp += tmpStr;
                        }
                        break;
                    }

                    // not support buffer add
                }
            }
        }

#else
        if (this->type == rvalue.getType())
        {
            if (this->type < 9 && typeB < 9)
            {
                switch (this->type)
                {
                case ETYPE_UINT8:
                    tmp = this->data.u8 * rvalue.getUint8();
                    break;
                case ETYPE_INT8:
                    tmp = this->data.i8 * rvalue.getInt8();
                    break;
                case ETYPE_UINT16:
                    tmp = this->data.u16 * rvalue.getUint16();
                    break;
                case ETYPE_INT16:
                    tmp = this->data.i16 * rvalue.getInt16();
                    break;
                case ETYPE_UINT32:
                    tmp = this->data.u32 * rvalue.getUint32();
                    break;
                case ETYPE_INT32:
                    tmp = this->data.i32 * rvalue.getInt32();
                    break;
                case ETYPE_UINT64:
                    tmp = this->data.u64 * rvalue.getUint64();
                    break;
                case ETYPE_INT64:
                    tmp = this->data.i64 * rvalue.getInt64();
                    break;
                case ETYPE_FLOAT:
                    tmp = this->data.f * rvalue.getFloat();
                    break;
                case ETYPE_DOUBLE:
                    tmp = this->data.d * rvalue.getDouble();
                    break;
                }
            }
        }
#endif
        return tmp;
    }

    // /
    uint8_t operator/(const uint8_t &n) const
    {
        return (uint8_t)(this->getUniversalDouble() / n);
    }
    int8_t operator/(const int8_t &n) const
    {
        return (int8_t)(this->getUniversalDouble() / n);
    }

    uint16_t operator/(const uint16_t &n) const
    {
        return (uint16_t)(this->getUniversalDouble() / n);
    }
    int16_t operator/(const int16_t &n) const
    {
        return (int16_t)(this->getUniversalDouble() / n);
    }

    uint32_t operator/(const uint32_t &n) const
    {
        return (uint32_t)(this->getUniversalDouble() / n);
    }
    int32_t operator/(const int32_t &n) const
    {
        return (int32_t)(this->getUniversalDouble() / n);
    }

    uint64_t operator/(const uint64_t &n) const
    {
        return (uint64_t)(this->getUniversalDouble() / n);
    }
    int64_t operator/(const int64_t &n) const
    {
        return (int64_t)(this->getUniversalDouble() / n);
    }

    double operator/(const double &n) const
    {
        return (double)(this->getUniversalDouble() / n);
    }

    // Element / Element
    Element operator/(const Element &rvalue) const
    {
        Element tmp;
        auto typeB = rvalue.getType();

        if (this->type == rvalue.getType())
        {
            if (this->type < 9 && typeB < 9)
            {
                switch (this->type)
                {
                case ETYPE_UINT8:
                    tmp = this->data.u8 / rvalue.getUint8();
                    break;
                case ETYPE_INT8:
                    tmp = this->data.i8 / rvalue.getInt8();
                    break;
                case ETYPE_UINT16:
                    tmp = this->data.u16 / rvalue.getUint16();
                    break;
                case ETYPE_INT16:
                    tmp = this->data.i16 / rvalue.getInt16();
                    break;
                case ETYPE_UINT32:
                    tmp = this->data.u32 / rvalue.getUint32();
                    break;
                case ETYPE_INT32:
                    tmp = this->data.i32 / rvalue.getInt32();
                    break;
                case ETYPE_UINT64:
                    tmp = this->data.u64 / rvalue.getUint64();
                    break;
                case ETYPE_INT64:
                    tmp = this->data.i64 / rvalue.getInt64();
                    break;
                case ETYPE_FLOAT:
                    tmp = this->data.f / rvalue.getFloat();
                    break;
                case ETYPE_DOUBLE:
                    tmp = this->data.d / rvalue.getDouble();
                    break;
                }
            }
        }

        return tmp;
    }

    // +=
    Element &operator+=(const uint8_t &n)
    {
        (*this) = (uint8_t)((*this) + n);
        return *this;
    }
    Element &operator+=(const int8_t &n)
    {
        (*this) = (int8_t)((*this) + n);
        return *this;
    }
    Element &operator+=(const uint16_t &n)
    {
        (*this) = (uint16_t)((*this) + n);
        return *this;
    }
    Element &operator+=(const int16_t &n)
    {
        (*this) = (int16_t)((*this) + n);
        return *this;
    }
    Element &operator+=(const uint32_t &n)
    {
        (*this) = (uint32_t)((*this) + n);
        return *this;
    }
    Element &operator+=(const int32_t &n)
    {
        (*this) = (int32_t)((*this) + n);
        return *this;
    }
    Element &operator+=(const uint64_t &n)
    {
        (*this) = (uint64_t)((*this) + n);
        return *this;
    }
    Element &operator+=(const int64_t &n)
    {
        (*this) = (int64_t)((*this) + n);
        return *this;
    }
    Element &operator+=(const double &n)
    {
        (*this) = (double)((*this) + n);
        return *this;
    }
    Element &operator+=(const float &n)
    {
        return this->operator+=((double)n);
    }

    Element &operator+=(const Element &e)
    {
        (*this) = (*this) + e;
        return (*this);
    }

    Element &operator+=(const Element *e)
    {
        (*this) = (*this) + (*e);
        return (*this);
    }

    Element &operator+=(const char *str)
    {
        if (strlen(str))
        {
            if (this->type == ETYPE_STRING)
            {
                this->_setString((this->getString() + str).c_str());
            }
        }
        return (*this);
    }

    const char *operator+=(const String &str)
    {
        if (str.length())
        {
            if (this->type == ETYPE_STRING)
            {
                this->_setString((this->getString() + str).c_str());
            }
        }
        return this->c_str();
    }

    const char *operator+=(const std::string &str)
    {
        if (str.length())
        {
            if (this->type == ETYPE_STRING)
            {
                this->_setString((this->getString() + str.c_str()).c_str());
            }
        }
        return this->c_str();
    }

    const char *operator+=(const String *str)
    {
        if (str->length())
        {
            if (this->type == ETYPE_STRING)
            {
                this->_setString((this->getString() + str->c_str()).c_str());
            }
        }
        return this->c_str();
    }

    const char *operator+=(const std::string *str)
    {
        if (str->length())
        {
            if (this->type == ETYPE_STRING)
            {
                this->_setString((this->getString() + str->c_str()).c_str());
            }
        }
        return this->c_str();
    }

    // -=

    Element &operator-=(const uint8_t &n)
    {
        (*this) = (uint8_t)((*this) - n);
        return *this;
    }
    Element &operator-=(const int8_t &n)
    {
        (*this) = (int8_t)((*this) - n);
        return *this;
    }
    Element &operator-=(const uint16_t &n)
    {
        (*this) = (uint16_t)((*this) - n);
        return *this;
    }
    Element &operator-=(const int16_t &n)
    {
        (*this) = (int16_t)((*this) - n);
        return *this;
    }
    Element &operator-=(const uint32_t &n)
    {
        (*this) = (uint32_t)((*this) - n);
        return *this;
    }
    Element &operator-=(const int32_t &n)
    {
        (*this) = (int32_t)((*this) - n);
        return *this;
    }
    Element &operator-=(const uint64_t &n)
    {
        (*this) = (uint64_t)((*this) - n);
        return *this;
    }
    Element &operator-=(const int64_t &n)
    {
        (*this) = (int64_t)((*this) - n);
        return *this;
    }
    Element &operator-=(const double &n)
    {
        (*this) = (double)((*this) - n);
        return *this;
    }
    Element &operator-=(const float &n)
    {
        return this->operator-=((double)n);
    }

    Element &operator-=(const Element &e)
    {
        (*this) = (*this) - e;
        return (*this);
    }

    Element &operator-=(const Element *e)
    {
        (*this) = (*this) - (*e);
        return (*this);
    }

    Element &operator-=(const char *str)
    {
        if (strlen(str))
        {
            if (this->type == ETYPE_STRING)
            {
                String tmp = this->getString();
                tmp.replace(str, "");
                this->_setString(tmp.c_str());
            }
        }
        return (*this);
    }

    const char *operator-=(const String &str)
    {
        if (str.length())
        {
            if (this->type == ETYPE_STRING)
            {
                String tmp = this->getString();
                tmp.replace(str, "");
                this->_setString(tmp.c_str());
            }
        }
        return this->c_str();
    }

    const char *operator-=(const std::string &str)
    {
        if (str.length())
        {
            if (this->type == ETYPE_STRING)
            {
                String tmp = this->getString();
                tmp.replace(str.c_str(), "");
                this->_setString(tmp.c_str());
            }
        }
        return this->c_str();
    }

    const char *operator-=(const String *str)
    {
        if (str->length())
        {
            if (this->type == ETYPE_STRING)
            {
                String tmp = this->getString();
                tmp.replace(str->c_str(), "");
                this->_setString(tmp.c_str());
            }
        }
        return this->c_str();
    }

    const char *operator-=(const std::string *str)
    {
        if (str->length())
        {
            if (this->type == ETYPE_STRING)
            {
                String tmp = this->getString();
                tmp.replace(str->c_str(), "");
                this->_setString(tmp.c_str());
            }
        }
        return this->c_str();
    }

    // *=

    Element &operator*=(const uint8_t &n)
    {
        (*this) = (uint8_t)((*this) * n);
        return *this;
    }
    Element &operator*=(const int8_t &n)
    {
        (*this) = (int8_t)((*this) * n);
        return *this;
    }
    Element &operator*=(const uint16_t &n)
    {
        (*this) = (uint16_t)((*this) * n);
        return *this;
    }
    Element &operator*=(const int16_t &n)
    {
        (*this) = (int16_t)((*this) * n);
        return *this;
    }
    Element &operator*=(const uint32_t &n)
    {
        (*this) = (uint32_t)((*this) * n);
        return *this;
    }
    Element &operator*=(const int32_t &n)
    {
        (*this) = (int32_t)((*this) * n);
        return *this;
    }
    Element &operator*=(const uint64_t &n)
    {
        (*this) = (uint64_t)((*this) * n);
        return *this;
    }
    Element &operator*=(const int64_t &n)
    {
        (*this) = (int64_t)((*this) * n);
        return *this;
    }
    Element &operator*=(const double &n)
    {
        (*this) = (double)((*this) * n);
        return *this;
    }
    Element &operator*=(const float &n)
    {
        return this->operator*=((double)n);
    }
    Element &operator*=(const Element &e)
    {
        (*this) = (*this) * e;
        return (*this);
    }

    Element &operator*=(const Element *e)
    {
        (*this) = (*this) * (*e);
        return (*this);
    }

    // /=

    Element &operator/=(const uint8_t &n)
    {
#ifdef AUTO_EXTEND_DATA_RANGE
        (*this) = (double)(this->getUniversalDouble() / (double)n);
#else

        (*this) = (uint8_t)((*this) / n);
#endif
        return *this;
    }
    Element &operator/=(const int8_t &n)
    {
#ifdef AUTO_EXTEND_DATA_RANGE
        (*this) = (double)(this->getUniversalDouble() / (double)n);
#else

        (*this) = (int8_t)((*this) / n);
#endif
        return *this;
    }

    Element &operator/=(const uint16_t &n)
    {
#ifdef AUTO_EXTEND_DATA_RANGE
        (*this) = (double)(this->getUniversalDouble() / (double)n);
#else

        (*this) = (uint16_t)((*this) / n);
#endif
        return *this;
    }
    Element &operator/=(const int16_t &n)
    {
#ifdef AUTO_EXTEND_DATA_RANGE
        (*this) = (double)(this->getUniversalDouble() / (double)n);
#else

        (*this) = (int16_t)((*this) / n);
#endif
        return *this;
    }

    Element &operator/=(const uint32_t &n)
    {
#ifdef AUTO_EXTEND_DATA_RANGE
        (*this) = (double)(this->getUniversalDouble() / (double)n);
#else

        (*this) = (uint32_t)((*this) / n);
#endif
        return *this;
    }
    Element &operator/=(const int32_t &n)
    {
#ifdef AUTO_EXTEND_DATA_RANGE
        (*this) = (double)(this->getUniversalDouble() / (double)n);
#else

        (*this) = (int32_t)((*this) / n);
#endif
        return *this;
    }

    Element &operator/=(const uint64_t &n)
    {
#ifdef AUTO_EXTEND_DATA_RANGE
        (*this) = (double)(this->getUniversalDouble() / (double)n);
#else

        (*this) = (uint64_t)((*this) / n);
#endif
        return *this;
    }
    Element &operator/=(const int64_t &n)
    {
#ifdef AUTO_EXTEND_DATA_RANGE
        (*this) = (double)(this->getUniversalDouble() / (double)n);
#else

        (*this) = (int64_t)((*this) / n);
#endif
        return *this;
    }

    Element &operator/=(const Element &e)
    {
        (*this) = (*this) / e;
        return (*this);
    }

    Element &operator/=(const Element *e)
    {
        (*this) = (*this) / (*e);
        return (*this);
    }

    // ++
    Element &operator++()
    {
        if (this->type < 9)
        {
            if (this->type == ETYPE_FLOAT)
            {
                this->data.f += 1.0;
            }
            else if (this->type == ETYPE_DOUBLE)
            {
                this->data.d += 1.0;
            }
            else
            {
                (*this) += 1;
            }
        }
        else
        {
            if (this->type == ETYPE_STRING)
            {
                this->_setString((this->getString() + this->getString()).c_str());
            }
        }
        return (*this);
    }

    Element operator++(int)
    {
        Element tmp = *this;
        ++(*this);
        return tmp;
    }

    // --

    Element &operator--()
    {
        if (this->type < 9)
        {
            if (this->type == ETYPE_FLOAT)
            {
                this->data.f -= 1.0;
            }
            else if (this->type == ETYPE_DOUBLE)
            {
                this->data.d -= 1.0;
            }
            else
            {
                (*this) -= 1;
            }
        }
        else
        {
            if (this->type == ETYPE_STRING)
            {
                String tmp = this->getString();
                uint32_t length = tmp.length();
                if (length > 0)
                {
                    this->_setString(tmp.substring(1).c_str());
                }
            }
        }
        return (*this);
    }

    Element operator--(int)
    {
        Element tmp = *this;
        --(*this);
        return tmp;
    }

    // %
    Element operator%(const int32_t &rvalue) const
    {
        Element tmp;
        if (this->type < 9 && this->type != ETYPE_FLOAT && this->type != ETYPE_DOUBLE)
        {
            tmp = (int32_t)(this->data.i64 % rvalue);
        }
        return tmp;
    }

    Element operator%(const Element &rvalue) const
    {
        return this->operator%((int32_t)rvalue.getNumber());
    }

    Element operator%(const Element *rvalue) const
    {
        return this->operator%((int32_t)rvalue->getNumber());
    }

    //%=

    Element &operator%=(const int32_t &rvalue)
    {
        Element result = (*this) % rvalue;
        if (result.available())
        {
            this->reset(ETYPE_INT32);
            this->data.i32 = result.getInt32();
        }
        return (*this);
    }

    Element &operator%=(const Element &rvalue)
    {
        return this->operator%=(rvalue.getNumber());
    }

    Element &operator%=(const Element *rvalue)
    {
        return this->operator%=(rvalue->getNumber());
    }

    // <<

    inline Element operator<<(const uint8_t &rvalue) const
    {
        Element tmp;
        if (this->type < 9 && this->type != ETYPE_FLOAT && this->type != ETYPE_DOUBLE)
        {
            switch (this->type)
            {
            case ETYPE_UINT8:
                tmp = (uint8_t)this->data.u8 << rvalue;
                break;
            case ETYPE_INT8:
                tmp = (int8_t)this->data.i8 << rvalue;
                break;
            case ETYPE_UINT16:
                tmp = (uint16_t)this->data.u16 << rvalue;
                break;
            case ETYPE_INT16:
                tmp = (int16_t)this->data.i16 << rvalue;
                break;
            case ETYPE_UINT32:
                tmp = (uint32_t)this->data.u32 << rvalue;
                break;
            case ETYPE_INT32:
                tmp = (int32_t)this->data.i32 << rvalue;
                break;
            case ETYPE_UINT64:
                tmp = (uint64_t)this->data.u64 << rvalue;
                break;
            case ETYPE_INT64:
                tmp = (int64_t)this->data.i64 << rvalue;
                break;
            default:
                break;
            }
        }
        else
        {
            if (this->type == ETYPE_STRING)
            {
                if (strlen(this->c_str()) >= rvalue)
                {
                    tmp = this->getString().substring(rvalue);
                }
            }
        }
        return tmp;
    }

    inline Element operator<<(const Element &rvalue) const
    {
        return this->operator<<(rvalue.getUint8());
    }

    inline Element operator<<(const Element *rvalue) const
    {
        return this->operator<<(rvalue->getUint8());
    }

    // >>

    inline Element operator>>(const uint8_t &rvalue) const
    {
        Element tmp;
        if (this->type < 9 && this->type != ETYPE_FLOAT && this->type != ETYPE_DOUBLE)
        {
            switch (this->type)
            {
            case ETYPE_UINT8:
                tmp = (uint8_t)this->data.u8 >> rvalue;
                break;
            case ETYPE_INT8:
                tmp = (int8_t)this->data.i8 >> rvalue;
                break;
            case ETYPE_UINT16:
                tmp = (uint16_t)this->data.u16 >> rvalue;
                break;
            case ETYPE_INT16:
                tmp = (int16_t)this->data.i16 >> rvalue;
                break;
            case ETYPE_UINT32:
                tmp = (uint32_t)this->data.u32 >> rvalue;
                break;
            case ETYPE_INT32:
                tmp = (int32_t)this->data.i32 >> rvalue;
                break;
            case ETYPE_UINT64:
                tmp = (uint64_t)this->data.u64 >> rvalue;
                break;
            case ETYPE_INT64:
                tmp = (int64_t)this->data.i64 >> rvalue;
                break;
            default:
                break;
            }
        }
        else
        {
            if (this->type == ETYPE_STRING)
            {
                if (strlen(this->c_str()) >= rvalue)
                {
                    String t = this->getString();
                    tmp = t.substring(0, t.length() - rvalue);
                }
            }
        }
        return tmp;
    }

    inline Element operator>>(const Element &rvalue) const
    {
        return this->operator>>(rvalue.getUint8());
    }

    inline Element operator>>(const Element *rvalue) const
    {
        return this->operator>>(rvalue->getUint8());
    }

    // <<=

    Element &operator<<=(const uint8_t &rvalue)
    {
        (*this) = this->operator<<(rvalue);
        return (*this);
    }
    Element &operator<<=(const Element &rvalue)
    {
        (*this) = this->operator<<(rvalue.getUint8());
        return (*this);
    }
    Element &operator<<=(const Element *rvalue)
    {
        (*this) = this->operator<<(rvalue->getUint8());
        return (*this);
    }

    // >>=

    Element &operator>>=(const uint8_t &rvalue)
    {
        (*this) = this->operator>>(rvalue);
        return (*this);
    }
    Element &operator>>=(const Element &rvalue)
    {
        (*this) = this->operator>>(rvalue.getUint8());
        return (*this);
    }
    Element &operator>>=(const Element *rvalue)
    {
        (*this) = this->operator>>(rvalue->getUint8());
        return (*this);
    }

    // &

    /**
     * @brief universal bitwise and
     * @note float and double also do bitwise and
     *
     * @attention check available before using it
     *
     * @param rvalue
     * @return Element
     */
    inline Element operator&(const Element &rvalue) const
    {
        if (!this->available() || !rvalue.available())
            return Element();

        switch (this->type)
        {
        case ETYPE_UINT8:
            return Element((uint8_t)(this->data.u8 & rvalue.getUint8()));
        case ETYPE_INT8:
            return Element((int8_t)(this->data.i8 & rvalue.getInt8()));
        case ETYPE_UINT16:
            return Element((uint16_t)(this->data.u16 & rvalue.getUint16()));
        case ETYPE_INT16:
            return Element((int16_t)(this->data.i16 & rvalue.getInt16()));
        case ETYPE_UINT32:
            return Element((uint32_t)(this->data.u32 & rvalue.getUint32()));
        case ETYPE_INT32:
            return Element((int32_t)(this->data.i32 & rvalue.getInt32()));
        case ETYPE_UINT64:
            return Element((uint64_t)(this->data.u64 & rvalue.getUint64()));
        case ETYPE_INT64:
            return Element((int64_t)(this->data.i64 & rvalue.getInt64()));
        default:
            break;
        }
        return Element();
    }

    // |

    /**
     * @brief universal bitwise or
     * @note float and double also do bitwise or
     *
     * @attention check available before using it
     *
     * @param rvalue
     * @return Element
     */
    inline Element operator|(const Element &rvalue) const
    {
        if (!this->available() || !rvalue.available())
            return Element();

        switch (this->type)
        {
        case ETYPE_UINT8:
            return Element((uint8_t)(this->data.u8 | rvalue.getUint8()));
        case ETYPE_INT8:
            return Element((int8_t)(this->data.i8 | rvalue.getInt8()));
        case ETYPE_UINT16:
            return Element((uint16_t)(this->data.u16 | rvalue.getUint16()));
        case ETYPE_INT16:
            return Element((int16_t)(this->data.i16 | rvalue.getInt16()));
        case ETYPE_UINT32:
            return Element((uint32_t)(this->data.u32 | rvalue.getUint32()));
        case ETYPE_INT32:
            return Element((int32_t)(this->data.i32 | rvalue.getInt32()));
        case ETYPE_UINT64:
            return Element((uint64_t)(this->data.u64 | rvalue.getUint64()));
        case ETYPE_INT64:
            return Element((int64_t)(this->data.i64 | rvalue.getInt64()));
        default:
            break;
        }
        return Element();
    }

    // ~

    /**
     * @brief bitwise inversion
     *
     * @attention check available before using it
     *
     * @return Element
     */
    inline Element operator~() const
    {
        switch (this->type)
        {
        case ETYPE_UINT8:
            return Element((uint8_t)(~(this->data.i64)));
        case ETYPE_INT8:
            return Element((int8_t)(~(this->data.i64)));
        case ETYPE_UINT16:
            return Element((uint16_t)(~(this->data.i64)));
        case ETYPE_INT16:
            return Element((int16_t)(~(this->data.i64)));
        case ETYPE_UINT32:
            return Element((uint32_t)(~(this->data.i64)));
        case ETYPE_INT32:
            return Element((int32_t)(~(this->data.i64)));
        case ETYPE_UINT64:
            return Element((uint64_t)(~(this->data.i64)));
        case ETYPE_INT64:
            return Element((int64_t)(~(this->data.i64)));
        default:
            break;
        }
        return Element();
    }

    // ^

    /**
     * @brief universal xor
     * @note float and double also do xor
     *
     * @attention check available before using it
     *
     * @param rvalue
     * @return Element
     */
    inline Element operator^(const Element &rvalue) const
    {
        if (!this->available() || !rvalue.available())
            return Element();

        switch (this->type)
        {
        case ETYPE_UINT8:
            return Element((uint8_t)(this->data.u8 ^ rvalue.getUint8()));
        case ETYPE_INT8:
            return Element((int8_t)(this->data.i8 ^ rvalue.getInt8()));
        case ETYPE_UINT16:
            return Element((uint16_t)(this->data.u16 ^ rvalue.getUint16()));
        case ETYPE_INT16:
            return Element((int16_t)(this->data.i16 ^ rvalue.getInt16()));
        case ETYPE_UINT32:
            return Element((uint32_t)(this->data.u32 ^ rvalue.getUint32()));
        case ETYPE_INT32:
            return Element((int32_t)(this->data.i32 ^ rvalue.getInt32()));
        case ETYPE_UINT64:
            return Element((uint64_t)(this->data.u64 ^ rvalue.getUint64()));
        case ETYPE_INT64:
            return Element((int64_t)(this->data.i64 ^ rvalue.getInt64()));
        default:
            break;
        }
        return Element();
    }

    // &&
    inline bool operator&&(const bool &rvalue) const
    {
        return this->available() &&
               (this->data.i64 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const uint8_t &rvalue) const
    {
        return this->available() &&
               (this->data.u8 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const int8_t &rvalue) const
    {
        return this->available() &&
               (this->data.i8 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const uint16_t &rvalue) const
    {
        return this->available() &&
               (this->data.u16 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const int16_t &rvalue) const
    {
        return this->available() &&
               (this->data.i16 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const uint32_t &rvalue) const
    {
        return this->available() &&
               (this->data.u32 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const int32_t &rvalue) const
    {
        return this->available() &&
               (this->data.i32 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const uint64_t &rvalue) const
    {
        return this->available() &&
               (this->data.u64 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const int64_t &rvalue) const
    {
        return this->available() &&
               (this->data.i64 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const float &rvalue) const
    {
        return this->available() &&
               (this->data.f != 0 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const double rvalue) const
    {
        return this->available() &&
               (this->data.d != 0 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               rvalue;
    }

    inline bool operator&&(const Element &rvalue) const
    {
        uint32_t bLength = rvalue.getRawBufferLength();
        ElementType bType = rvalue.getType();
        return this->available() &&
               (this->data.i64 ||
                (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) &&
               (rvalue.getInt64() ||
                (bLength > (bType == ETYPE_STRING ? 1 : 0)));
    }

    // ||

    inline bool operator||(const bool &rvalue) const
    {
        return this->available() && (this->data.i64 ||
                                     (this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0)) || rvalue);
    }

    inline bool operator||(const uint8_t &rvalue) const
    {
        return this->available() && (this->data.u8 ||
                                     ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    bool operator||(const int8_t &rvalue) const
    {
        return this->available() && (this->data.i8 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    inline bool operator||(const uint16_t &rvalue) const
    {
        return this->available() && (this->data.u16 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    inline bool operator||(const int16_t &rvalue) const
    {
        return this->available() && (this->data.i16 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    inline bool operator||(const uint32_t &rvalue) const
    {
        return this->available() && (this->data.u32 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    inline bool operator||(const int32_t &rvalue) const
    {
        return this->available() && (this->data.i32 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    inline bool operator||(const uint64_t &rvalue) const
    {
        return this->available() && (this->data.u64 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    inline bool operator||(const int64_t &rvalue) const
    {
        return this->available() && (this->data.i64 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    inline bool operator||(const float &rvalue) const
    {
        return this->available() && (this->data.f != 0 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    inline bool operator||(const double rvalue) const
    {
        return this->available() && (this->data.d != 0 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) || rvalue);
    }

    inline bool operator||(const Element &rvalue) const
    {
        uint32_t bLength = rvalue.getRawBufferLength();
        ElementType bType = rvalue.getType();
        return (this->available() &&
                rvalue.available()) &&
               (this->data.i64 || ((this->data.buffer.bufferLength > (this->type == ETYPE_STRING ? 1 : 0))) ||
                rvalue.getInt64() || ((bLength > (bType == ETYPE_STRING ? 1 : 0))));
    }

    // !

    /**
     * @brief logical not
     *
     * @attention this won't check available
     *
     * @return true
     * @return false
     */
    inline bool operator!() const
    {
        if (this->available())
        {
            switch (this->type)
            {
            case ETYPE_UINT8:
                return !this->data.u8;
            case ETYPE_INT8:
                return !this->data.i8;
            case ETYPE_UINT16:
                return !this->data.u16;
            case ETYPE_INT16:
                return !this->data.i16;
            case ETYPE_UINT32:
                return !this->data.u32;
            case ETYPE_INT32:
                return !this->data.i32;
            case ETYPE_UINT64:
                return !this->data.u64;
            case ETYPE_INT64:
                return !this->data.i64;
            case ETYPE_FLOAT:
                return this->data.f == 0;
            case ETYPE_DOUBLE:
                return this->data.d == 0;
            case ETYPE_STRING:
                return !((this->data.buffer.p)[0]);
            case ETYPE_BUFFER:
                return !this->data.buffer.bufferLength;
            default:
                return false;
            }
        }
        return true;
    }

    // []
    /**
     * @brief get uint8_t data from buffer or string
     *
     * @attention the return value is uint16_t, it doesn't mean
     * data range from -32768 to 32767.
     * return -2 means error, or it will return one byte data
     *
     * @note index from head if positive number given, or index from tail
     *
     * @param index index in int32_t
     * @return int16_t
     */
    inline int16_t operator[](const int32_t &index) const
    {
        if (this->type == ETYPE_BUFFER || this->type == ETYPE_STRING)
        {
            if (index < this->data.buffer.bufferLength)
            {
                if (index > 0)
                {
                    return (this->data.buffer.p)[index];
                }
                else
                {
                    return (this->data.buffer.p)[this->data.buffer.bufferLength - index];
                }
            }
        }
        return -2;
    }

    /**
     * @brief get uint8_t data from buffer or string
     *
     * @attention the return value is uint16_t, it doesn't mean
     * data range from -32768 to 32767.
     * return -2 means error, or it will return one byte data
     *
     * @note index from head if positive number given, or index from tail
     *
     * @param index index in Element
     * @return int16_t
     */
    inline int16_t operator[](const Element &index) const
    {
        if (this->type == ETYPE_BUFFER || this->type == ETYPE_STRING)
        {
            if (index.available())
            {
                int32_t i = index.getInt32();
                if (i < this->data.buffer.bufferLength)
                {
                    if (i > 0)
                    {
                        return (this->data.buffer.p)[i];
                    }
                    else
                    {
                        return (this->data.buffer.p)[this->data.buffer.bufferLength - i];
                    }
                }
            }
        }
        return -2;
    }

    // &=

    inline uint8_t operator&=(const uint8_t &rvalue)
    {
        this->reset(ETYPE_UINT8);
        this->data.u8 &= rvalue;
        return this->data.u8;
    }
    inline int8_t operator&=(const int8_t &rvalue)
    {
        this->reset(ETYPE_INT8);
        this->data.i8 &= rvalue;
        return this->data.i8;
    }
    inline uint16_t operator&=(const uint16_t &rvalue)
    {
        this->reset(ETYPE_UINT16);
        this->data.u16 &= rvalue;
        return this->data.u16;
    }
    inline int16_t operator&=(const int16_t &rvalue)
    {
        this->reset(ETYPE_INT16);
        this->data.i16 &= rvalue;
        return this->data.i16;
    }
    inline uint32_t operator&=(const uint32_t &rvalue)
    {
        this->reset(ETYPE_UINT32);
        this->data.u32 &= rvalue;
        return this->data.u32;
    }
    inline int32_t operator&=(const int32_t &rvalue)
    {
        this->reset(ETYPE_INT32);
        this->data.i32 &= rvalue;
        return this->data.i32;
    }
    inline uint64_t operator&=(const uint64_t &rvalue)
    {
        this->reset(ETYPE_UINT64);
        this->data.u64 &= rvalue;
        return this->data.u64;
    }
    inline int64_t operator&=(const int64_t &rvalue)
    {
        this->reset(ETYPE_INT64);
        this->data.i64 &= rvalue;
        return this->data.i64;
    }
    inline Element &operator&=(const Element &rvalue)
    {
        this->data.i64 &= rvalue.getInt64();
        return *this;
    }

    // |=

    inline uint8_t operator|=(const uint8_t &rvalue)
    {
        this->reset(ETYPE_UINT8);
        this->data.u8 |= rvalue;
        return this->data.u8;
    }
    inline int8_t operator|=(const int8_t &rvalue)
    {
        this->reset(ETYPE_INT8);
        this->data.i8 |= rvalue;
        return this->data.i8;
    }
    inline uint16_t operator|=(const uint16_t &rvalue)
    {
        this->reset(ETYPE_UINT16);
        this->data.u16 |= rvalue;
        return this->data.u16;
    }
    inline int16_t operator|=(const int16_t &rvalue)
    {
        this->reset(ETYPE_INT16);
        this->data.i16 |= rvalue;
        return this->data.i16;
    }
    inline uint32_t operator|=(const uint32_t &rvalue)
    {
        this->reset(ETYPE_UINT32);
        this->data.u32 |= rvalue;
        return this->data.u32;
    }
    inline int32_t operator|=(const int32_t &rvalue)
    {
        this->reset(ETYPE_INT32);
        this->data.i32 |= rvalue;
        return this->data.i32;
    }
    inline uint64_t operator|=(const uint64_t &rvalue)
    {
        this->reset(ETYPE_UINT64);
        this->data.u64 |= rvalue;
        return this->data.u64;
    }
    inline int64_t operator|=(const int64_t &rvalue)
    {
        this->reset(ETYPE_INT64);
        this->data.i64 |= rvalue;
        return this->data.i64;
    }
    inline Element &operator|=(const Element &rvalue)
    {
        this->data.i64 |= rvalue.getInt64();
        return *this;
    }

    // ^=
    inline uint8_t operator^=(const uint8_t &rvalue)
    {
        this->reset(ETYPE_UINT8);
        this->data.u8 ^= rvalue;
        return this->data.u8;
    }
    inline int8_t operator^=(const int8_t &rvalue)
    {
        this->reset(ETYPE_INT8);
        this->data.i8 ^= rvalue;
        return this->data.i8;
    }
    inline uint16_t operator^=(const uint16_t &rvalue)
    {
        this->reset(ETYPE_UINT16);
        this->data.u16 ^= rvalue;
        return this->data.u16;
    }
    inline int16_t operator^=(const int16_t &rvalue)
    {
        this->reset(ETYPE_INT16);
        this->data.i16 ^= rvalue;
        return this->data.i16;
    }
    inline uint32_t operator^=(const uint32_t &rvalue)
    {
        this->reset(ETYPE_UINT32);
        this->data.u32 ^= rvalue;
        return this->data.u32;
    }
    inline int32_t operator^=(const int32_t &rvalue)
    {
        this->reset(ETYPE_INT32);
        this->data.i32 ^= rvalue;
        return this->data.i32;
    }
    inline uint64_t operator^=(const uint64_t &rvalue)
    {
        this->reset(ETYPE_UINT64);
        this->data.u64 ^= rvalue;
        return this->data.u64;
    }
    inline int64_t operator^=(const int64_t &rvalue)
    {
        this->reset(ETYPE_INT64);
        this->data.i64 ^= rvalue;
        return this->data.i64;
    }
    inline Element &operator^=(const Element &rvalue)
    {
        this->data.i64 ^= rvalue.getInt64();
        return *this;
    }

    // operators

    String replace(const char *target, const char *newContent)
    {
        if (this->type == ETYPE_STRING)
        {
            String tmp = this->getString();
            tmp.replace(target, newContent);
            this->_setString(tmp.c_str());
            return tmp;
        }
        return "";
    }

    String replace(const String &target, const char *newContent)
    {
        return this->replace(target.c_str(), newContent);
    }

    inline const char *
    c_str() const
    {
        if (this->type == ETYPE_STRING)
            return (const char *)(this->data.buffer.p);
        return "";
    }

    inline int indexOf(const String &target) const
    {
        if (this->type == ETYPE_STRING)
            return this->getString().indexOf(target);
        return -2;
    }
    inline int indexOf(const char &target) const
    {
        if (this->type == ETYPE_STRING)
            for (uint32_t i = 0; i < this->data.buffer.bufferLength; ++i)
                if (target == (this->data.buffer.p)[i])
                    return i;

        return -2;
    }

    inline int lastIndexOf(const String &target) const
    {
        if (this->type == ETYPE_STRING)
            return this->getString().lastIndexOf(target);
        return -2;
    }
    inline int lastIndexOf(const char &target) const
    {
        if (this->type == ETYPE_STRING)
            for (uint32_t i = this->data.buffer.bufferLength - 1; i >= 0; --i)
                if (target == (this->data.buffer.p)[i])
                    return i;
        return -2;
    }

    /**
     * @brief deconstructor, it will clean buffer automatically
     * 析构函数， 它会调用清理缓存的函数
     */
    ~Element()
    {
        // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "object de, address: %x", this);

        // clear buffer if buffer is copied from other places
        // 如果buffer是从其他位置拷贝的就清除buffer
        if (this->copiedBuffer)
        {
            this->clearBuffer();
        }
    }

    /**
     * @brief get current type of data that object stored
     * 获取当前对象存储的数据类型
     *
     * @param isNumber will return NUMBER if give argument a true, if current object hold a number
     * 参数传true如果当前对象存储了数字只会告诉你类型是NUMBER
     *
     * @return the data type that current object stored
     * 当前对象中存储的数据的数据类型
     */
    ElementType getType(bool isNumber = false) const
    {
        return isNumber ? ((this->type < 9)
                               ? ETYPE_NUMBER
                               : this->type)
                        : (this->type);
    }

    /**
     * @brief get number
     * 获取存储的数字
     *
     * @return stored number
     */
    inline uint8_t getUint8() const { return (uint8_t)this->data.u8; }
    inline int8_t getInt8() const { return (uint8_t)this->data.i8; }
    inline uint16_t getUint16() const { return (uint16_t)this->data.u16; }
    inline int16_t getInt16() const { return (uint16_t)this->data.i16; }
    inline uint32_t getUint32() const { return (uint32_t)this->data.u32; }
    inline int32_t getInt32() const { return (uint32_t)this->data.i32; }
    inline uint64_t getUint64() const { return this->data.u64; }
    inline int64_t getInt64() const { return this->data.i64; }
    inline float getFloat() const { return this->data.f; }
    inline double getDouble() const { return this->data.d; }

    /**
     * @brief get string
     * will return hex string if current object stored uint8 array(not string)
     * you could remove it if you like
     *
     * 获取存储的字符串
     * 如果当前对象存储的是一个二进制数组，则默认会返回它的十六进制字符串
     * 不想要的话可以去掉
     *
     * @return stored string
     */
    inline String getString() const
    {
        // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "get string");
        return (this->type == ETYPE_STRING) ? (this->data.buffer.p ? (String((const char *)this->data.buffer.p)) : (String("")))
                                            : ((this->type == ETYPE_BUFFER) ? (this->getHex()) : (String("")));
    }

    /**
     * @brief the following two functions have same behavior when storing uint8 array
     * but the first one will check type equals to uint8 array or not
     * or it will return nullptr
     *
     * 下面这俩函数当 当前对象存储的是二进制数组的时候没有区别
     * 但是第一个函数会检查对象存储的数据类型，不是二进制数组类型会返回空指针
     * (其实是一开始字符串用String存的，所以有很多地方用了getUint8Array，后来才改成都用buffer存的，
     * 但是这个我才不告诉你)
     *
     * @param outLen length of uint8 array stored in object 对象存储的二进制数组的长度
     *
     * @return stored uint8 array 存储的二进制数组
     */
    inline uint8_t *getUint8Array(uint32_t *outLen = nullptr) const
    {
        if (this->type == ETYPE_BUFFER)
        {
            if (outLen)
                (*outLen) = this->data.buffer.bufferLength;
            return (uint8_t *)(this->data.buffer.p);
        }
        else
        {
            if (outLen)
                (*outLen) = 0;
            return nullptr;
        }
    }

    /**
     * @brief convert data type in current object from String(hex format) to uint8 array
     * will update buffer stored in current object
     *
     * 把当前对象存储的16进制字符串转换为二进制数组，更新当前对象的buffer
     *
     * @return true: 当前对象存储了格式正确的16进制字符串且转换无错误
     * @return false: 当前对象存储了错误的16进制字符串或转换过程中存在错误
     */
    bool convertHexStringIntoUint8Array()
    {
        // check type and length 检查类型和长度
        if (this->type != ETYPE_STRING || (!this->data.buffer.p) || (!this->data.buffer.bufferLength))
            return false;

        // get original length
        // 获取原始长度
        uint32_t originalLength = this->getString().length();

        // the length of hex string must be mutiple times of 2
        // 16进制字符串的长度必须是2的整数倍
        if (originalLength % 2)
            return false;

        // length after convert
        // 转换后的长度
        uint32_t len = originalLength / 2;

        int tmp = 0;

        // start converting
        // 开始转换
        for (uint32_t i = 0, k = 0; i < originalLength; i += 2)
        {
            sscanf((const char *)((this->data.buffer.p) + i), "%02x", &tmp);
            (this->data.buffer.p)[k] = (uint8_t)tmp;
            ++k;
        }

        // modify type 修改类型
        this->type = ETYPE_BUFFER;

        // modify length 修改长度
        this->data.buffer.bufferLength = len;

        return true;
    }

    /**
     * @brief this function get raw buffer anyway
     * 这个函数无论如何都会返回缓存的指针
     *
     * @param outLen length of uint8 array stored in object 存储的数组的长度
     *
     * @return uint8 array 存储的数组
     */
    inline uint8_t *getRawBuffer(uint32_t *outLen = nullptr) const
    {
        if (outLen)
            (*outLen) = this->data.buffer.bufferLength;

        return (this->data.buffer.p);
    }

    /**
     * @brief get length of uint8 array if current object stroed uint8 array
     * will return 0 if object stored other type data
     *
     * 获取二进制数组的长度，当且仅当存储的数据类型是二进制数组的时候才返回长度，否则返回0
     *
     * @return length of uint8 array 存储的数组的长度
     */
    inline uint32_t getU8aLen() const { return this->type == ETYPE_BUFFER ? this->data.buffer.bufferLength : 0; }

    /**
     * @brief more simply way to get number, return int64_t
     *
     * 更方便的方式去获取存储的数字, 返回类型为int64_t
     *
     * @return stored number 存储的数字
     */
    inline int64_t getNumber() const
    {
        if (this->type < 9)
        {
            switch (this->type)
            {
            case ETYPE_UINT8:
                return (int64_t)this->data.u8;
            case ETYPE_INT8:
                return (int64_t)this->data.i8;
            case ETYPE_UINT16:
                return (int64_t)this->data.u16;
            case ETYPE_INT16:
                return (int64_t)this->data.i16;
            case ETYPE_UINT32:
                return (int64_t)this->data.u32;
            case ETYPE_INT32:
                return (int64_t)this->data.i32;
            case ETYPE_UINT64:
                return (int64_t)this->data.u64;
            case ETYPE_INT64:
                return (int64_t)this->data.i64;
            case ETYPE_FLOAT:
                return (int64_t)this->data.f;
            case ETYPE_DOUBLE:
                return (int64_t)this->data.d;
            }
        }
        return 0;
    }

    /**
     * @brief directly set raw uint8 array, attention attached
     * 直接设置/更改当前对象存储一个二进制数组, 请阅读注意事项
     *
     * @attention THIS FUNCTION WILL CLEAN BUFFER AUTOMATICALLY
     * 这个函数会清理缓存(如果对象原来存储了二进制数组或者字符串，那就被清理了)
     *
     * @param data new uint8 array pointer 新的二进制数组指针
     * @param length length of new uint8 array 新的二进制数组的长度
     */
    inline bool setUint8Array(uint8_t *data, uint64_t length)
    {
        if (!data || !length)
        {
            return false;
        }
        this->reset(ETYPE_BUFFER);
        return this->_copyBuffer(data, length, 0);
    }

    /**
     * @brief get hex string if this object stored raw uint8 array
     * will return String if current object holds a string
     * you could remove it if you like
     *
     * 获取当前对象中存储的二进制数组的十六进制字符串
     * 如果存的就是个字符串就会直接返回这个字符串
     * (其实是一开始调试的时候sha256直接存的十六进制字符串，后来才改成32字节二进制数组，
     * 我肯定是不会告诉你这个的)
     *
     * @param lowerCase lower case or upper case of output hex string
     * 16进制字符串的大小写
     *
     * @return hex string of uint8 array 二进制数组的16进制字符串
     */
    String getHex(bool lowerCase = true) const
    {
        if (this->type != ETYPE_BUFFER)
        {
            if (this->type == ETYPE_STRING)
            {
                // sometimes current object already stored a hex string, then return it directly
                // you could delete this part if you like
                // 有时候当前对象存储的已经是个16进制字符串就直接返回它
                // 不想要这部分可以删除
                return this->getString();
            }
            else
            {
                // return empty string when current object stored data is neither uint8 array or string
                // 如果当前对象存储的既不是字节数组或字符串就返回空字符串
                return String("");
            }
        }

        // define return length( length of uint8 array * 2 + 1('\0'))
        // 定义返回长度(字节数组的长度 * 2 + 1('\0'))
        uint32_t length = (this->data.buffer.bufferLength * 2) + 1;

        // define buffer 定义buffer
        // attention: this buffer is decleared and defined in stack
        // so you shouldn't convert a large uint8 array
        // 注意: 这个缓存声明定义在栈，所以不要转换巨大的字节数组
        char buf[length] = {0};

        // format 格式
        char hexCase[5] = {'%', '0', '2', 'x', '\0'};

        if (!lowerCase)
        {
            hexCase[3] = 'X';
        }

        // genereate hex string
        // 生成16进制字符串
        for (uint32_t i = 0; i < this->data.buffer.bufferLength; i++)
        {
            sprintf(buf + (i * 2), hexCase, (this->data.buffer.p)[i]);
        }

        return String(buf);
    }

    /**
     * @brief get length of raw buffer whether object stored string or uint8 array
     * 直接获取原始buffer的长度
     *
     * @return length of raw uint8 array 缓存长度
     */
    uint64_t getRawBufferLength() const
    {
        return this->data.buffer.bufferLength;
    }

    

    /**
     * @brief encode with Base64
     * 用 Base64 编码
     *
     * @note this function only fits in buffer and string
     * 仅字符串和二进制数组适用此函数
     *
     * @param replaceOrigin replace origin or not; 是否替换原有内容
     * @return String Base64 encoded
     */
    String toBase64(bool replaceOrigin = false)
    {
        String b64 = "";
        if (this->type == ETYPE_STRING || this->type == ETYPE_BUFFER)
        {
            b64 = mycrypto::Base64::base64Encode(this->data.buffer.p,
                                                 (this->type == ETYPE_STRING ? this->data.buffer.bufferLength - 1
                                                                             : this->data.buffer.bufferLength));
            if (replaceOrigin)
            {
                this->_setString(b64);
            }
        }
        return b64;
    }

    /**
     * @brief deocde with Base64
     * 用 Base64 解码
     *
     * @note this function only fits in string
     * 仅字符串适用此函数
     *
     * @param replaceOrigin replace origin or not; 是否替换原有内容
     * @return String Base64 decoded
     */
    String fromBase64(bool replaceOrigin = false)
    {
        String db64 = "";
        if (this->type == ETYPE_STRING)
        {
            db64 = mycrypto::Base64::base64Decode(this->getString());
            if (replaceOrigin)
            {
                this->_setString(db64);
            }
        }
        return db64;
    }

    /**
     * @brief calculate SHA256 of content
     *
     * @attention only string and buffer can do this process,
     * otherwise will return null pointer.
     * developer MUST call delete to the pointer after using it.
     *
     * 只有字符串和字节数组才能使用此函数，否则会返回空指针。
     * 开发者【需要】自己手动 delete 以释放内存空间
     *
     * @return uint8_t* pointer to result, 32 bytes long; 指向结果数组的指针，32字节
     */
    uint8_t *SHA256()
    {
        if (this->type == ETYPE_STRING || this->type == ETYPE_BUFFER)
        {
            uint8_t *buf = new (std::nothrow) uint8_t[32];
            if (!buf)
            {
                return buf;
            }
            bzero(buf, 32);

            mycrypto::SHA::sha256((this->type == ETYPE_STRING ? (uint8_t *)(this->c_str()) : (this->data.buffer.p)),
                                  this->type == ETYPE_STRING ? this->data.buffer.bufferLength - 1 : this->data.buffer.bufferLength, buf);

            return buf;
        }
        return nullptr;
    }

    /**
     * @brief get SHA256 of content in hex string format;
     * 获取内容的SHA256, 16进制字符串格式
     *
     * @attention will return empty string if type of content are not string and buffer
     * 如果内容不是字符串和二进制数组会返回空字符串
     *
     * @param replaceOrigin will replace origin if set to true
     * 如果参数为true则会直接替换掉原本的内容
     *
     * @return String SHA256 hex string
     */
    String getSHA256HexString(bool replaceOrigin = false)
    {
        String hash = "";
        if (this->type == ETYPE_STRING || this->type == ETYPE_BUFFER)
        {
            hash = mycrypto::SHA::sha256(
                this->data.buffer.p,
                this->type == ETYPE_STRING ? this->data.buffer.bufferLength - 1 : this->data.buffer.bufferLength);

            if (hash.length() && replaceOrigin)
            {
                this->_setString(hash);
            }
        }

        return hash;
    }

    /**
     * @brief encrypt content with AES-256-CBC
     * 用AES 256 CBC 加密数据
     *
     * @note only string and buffer fit in this function
     * 只有字符串和二进制数组适用此函数
     *
     * @param key key, 32 bytes; 密匙, 32字节
     * @param iv iv 16 bytes; 初始化向量, 16字节
     *
     * @return true success; 成功
     * @return false failed; 失败
     */
    bool AES256_CBC(const char *key, const char *iv)
    {
        if ((!strlen(key) || !strlen(iv)) && (this->type == ETYPE_BUFFER || this->type == ETYPE_STRING))
        {
            return false;
        }

        uint32_t outLen = 0;

        uint8_t *cipher = mycrypto::AES::aes256CBCEncrypt(
            (const uint8_t *)key,
            (const uint8_t *)iv,
            this->data.buffer.p,
            (this->type == ETYPE_STRING ? this->data.buffer.bufferLength - 1 : this->data.buffer.bufferLength),
            &outLen);

        if (!outLen && !cipher)
        {
            return false;
        }

        this->_copyBuffer(cipher, outLen);
        delete cipher;
        cipher = nullptr;

        return true;
    }

    /**
     * @brief encode with AES-256-CBC and replace origin with hex string
     * 用AES-256-CBC加密，然后替换原有内容为加密结果的16进制字符串
     *
     * @param key key, 32 bytes; 密匙, 32字节
     * @param iv iv 16 bytes; 初始化向量, 16字节
     *
     * @return true success; 成功
     * @return false failed; 失败
     */
    bool toAES256CBCHexString(const char *key, const char *iv)
    {
        if (!this->AES256_CBC(key, iv))
        {
            return false;
        }
        return this->_setString(this->getHex());
    }

    /**
     * @brief encode with AES-256-CBC and replace origin with base64
     * 用AES-256-CBC加密，然后替换原有内容为加密结果的base64编码
     *
     * @param key key, 32 bytes; 密匙, 32字节
     * @param iv iv 16 bytes; 初始化向量, 16字节
     *
     * @return true success; 成功
     * @return false failed; 失败
     */
    bool toAES256CBCBase64(const char *key, const char *iv)
    {
        if (!this->AES256_CBC(key, iv))
        {
            return false;
        }
        return this->_setString(this->toBase64(true));
    }
    /**
     * @brief
     * clear buffer
     * 清理缓存的函数
     */
    void clearBuffer()
    {
        // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "buffer length: %lu", this->data.buffer.bufferLength);
        if ((this->type == ETYPE_STRING || this->type == ETYPE_BUFFER) && this->data.buffer.p && this->data.buffer.bufferLength)
        {

            // clear length 归零长度
            // this->data.buffer.bufferLength = 0;

            // clear buffer 清除buffer
            delete this->data.buffer.p;

            // reset pointer 重置指针
            // this->data.buffer.p = nullptr;
        }
        bzero(&(this->data), sizeof(ElementData));
    }
};

typedef std::vector<Element *> Elements;
typedef std::function<void(uint8_t *output, uint64_t length, bool *isBufferDeleted)> createArrayBufferCallback;
typedef std::function<void(Elements *output)> decodeArrayBufferCallback;

/**
 * @brief
 * this class is a static class
 * 这是个静态类
 */
class ArrayBuffer
{
public:
    /**
     * @brief this function will create an uint8 array using a vector provided
     * 这个函数会使用你提供的vector构造一个二进制数组
     *
     * @param elements a vector contains pointers of class Element 一个装有Element对象指针的容器
     *
     * @param outLen length of output uint8 array 输出的二进制数组的长度
     *
     * @return generated pointer to buffer 生成的二进制数组指针
     */
    static uint8_t *createArrayBuffer(Elements *elements,
                                      uint32_t *outLen)
    {
        // check vector size
        // 检查容器大小
        if (elements->size() <= 0)
        {
            ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "empty container");
            *(outLen) = 0;
            return nullptr;
        }

        // declare the length of output
        // 声明输出二进制数组的大小
        uint64_t bufferLength = 0;

        // set iterator begin and end
        std::vector<Element *>::iterator it = elements->begin();
        std::vector<Element *>::iterator end = elements->end();

        // calculate buffer length
        // 计算需要的缓冲区的长度
        for (; it != end; ++it)
        {
            switch ((*it)->getType())
            {
            case ETYPE_UINT8:
            case ETYPE_INT8:
                // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "UINT8");
                bufferLength += 2;
                break;
            case ETYPE_UINT16:
            case ETYPE_INT16:
                // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "UINT16");
                bufferLength += 3;
                break;
            case ETYPE_UINT32:
            case ETYPE_INT32:
                // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "UINT32");
                bufferLength += 5;
                break;
            case ETYPE_UINT64:
            case ETYPE_INT64:
                // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "UINT64");
                bufferLength += 9;
                break;
            case ETYPE_FLOAT:
                bufferLength += 5;
                break;
            case ETYPE_DOUBLE:
                bufferLength += 9;
                break;
            case ETYPE_STRING:
                // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "STRING");
                bufferLength += 5;
                bufferLength += (*it)->getRawBufferLength() - 1;
                break;
            case ETYPE_BUFFER:
                // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "U8A");
                bufferLength += 5;
                bufferLength += (*it)->getU8aLen();
                break;
            case ETYPE_VOID:
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "ETYPE_VOID type");
                (*outLen) = 0;
                return nullptr;
                /**
                 * @brief above part could changes to:
                 *
                 * break;
                 *
                 * this will omit current element object
                 *
                 */
                break;
            default:
                /**
                 * @brief unknown type detected, this may cause by other errors in code
                 *
                 */
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "unknown type");
                (*outLen) = 0;
                return nullptr;
            }
        }

        // allocate buffer
        // 分配内存
        uint8_t *buf = new (std::nothrow) uint8_t[bufferLength];
        if (!buf)
        {
            // will return null pointer if memory allocate failed
            // 如果内存分配失败将返回空指针
            ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "memory allocate failed, buffer length: % llu", bufferLength);
            (*outLen) = 0;
            return nullptr;
        }
        bzero(buf, bufferLength);

        // reset iterator position
        // 重置迭代器指针位置
        it = elements->begin();

        // offset of output buffer
        // 输出的缓冲区的偏移量
        uint64_t k = 0;

        uint16_t a = 0;
        uint32_t b = 0;
        uint64_t c = 0;
        uint8_t *e = nullptr;
        float f = 0.0f;
        double d = 0.0f;

        // start to fill data
        // 从这里开始装填数据
        // 其实用一个字节来表示类型太浪费了，但是比较方便

        for (; it != end; ++it)
        {
            // mark will took 1 byte
            // 标志位会占用一个字节
            switch ((*it)->getType())
            {
            case ETYPE_UINT8:
                // 2 bytes total
                // 一共2字节
                // set mark
                // 设置标志位
                // 下面一样
                buf[k] = MARK_UINT8;
            case ETYPE_INT8:
                buf[k] = buf[k] ? buf[k] : MARK_INT8;

                // set data
                buf[k + 1] = (*it)->getUint8();

                // move offset
                k += 2;
                break;

            case ETYPE_UINT16:
                // 3 bytes total
                // set mark
                buf[k] = MARK_UINT16;
            case ETYPE_INT16:
                buf[k] = buf[k] ? buf[k] : MARK_INT16;

                // get number
                a = (*it)->getUint16();

                // fill data
                buf[k + 1] = (a & 0xff00U) >> 8;
                buf[k + 2] = (a & 0x00ffU);

                // move offset
                k += 3;

                a = 0;
                break;
            case ETYPE_UINT32:
                // 5 bytes total
                // set mark
                buf[k] = MARK_UINT32;
            case ETYPE_INT32:
                buf[k] = buf[k] ? buf[k] : MARK_INT32;

                // get number
                b = (*it)->getUint32();

                // fill data
                buf[k + 1] = (b & 0xff000000U) >> 24;
                buf[k + 2] = (b & 0x00ff0000U) >> 16;
                buf[k + 3] = (b & 0x0000ff00U) >> 8;
                buf[k + 4] = (b & 0x000000ffU);

                // move offset
                k += 5;

                b = 0;
                break;
            case ETYPE_FLOAT:
                buf[k] = MARK_FLOAT;

                // get number
                f = (*it)->getFloat();
                ++k;

                memcpy(buf + k, &f, 4);

                // move offset
                k += 4;

                f = 0;
                break;
            case ETYPE_UINT64:
                // 9 bytes total
                // set mark
                buf[k] = MARK_UINT64;
            case ETYPE_INT64:
                buf[k] = buf[k] ? buf[k] : MARK_INT64;

                // get number
                c = (*it)->getUint64();

                // fill data
                for (int i = 8; i > 0; --i)
                {
                    buf[k + i] = c & 0xff;
                    c >>= 8;
                }

                // move offset
                k += 9;

                c = 0;

                break;
            case ETYPE_DOUBLE:
                buf[k] = MARK_DOUBLE;

                // get number
                d = (*it)->getDouble();
                ++k;

                memcpy(buf + k, &d, 8);

                // move offset
                k += 8;

                d = 0;
                break;
            case ETYPE_STRING:
                // 5 + length of string total
                {
                    // get length of string
                    // 获取字符串的长度
                    b = (*it)->getRawBufferLength() - 1;

                    // set mark
                    buf[k] = MARK_STRING;

                    // set length of string
                    // 填充字符串的长度
                    buf[k + 1] = (b & 0xff000000U) >> 24;
                    buf[k + 2] = (b & 0x00ff0000U) >> 16;
                    buf[k + 3] = (b & 0x0000ff00U) >> 8;
                    buf[k + 4] = (b & 0x000000ffU);

                    // move offset
                    // 修改偏移量
                    k += 5;

                    // copy buffer
                    // 拷贝
                    memcpy(buf + k, (*it)->getRawBuffer(), b);

                    // move offset
                    k += b;

                    // reset variables
                    b = 0;
                }
                break;
            case ETYPE_BUFFER:
                // 5 + length of buffer total

                // get buffer pointer and length of buffer to "b"
                e = (*it)->getUint8Array(&b);

                // set mark
                buf[k] = MARK_BUFFER;

                // set length of buffer
                buf[k + 1] = (b & 0xff000000U) >> 24;
                buf[k + 2] = (b & 0x00ff0000U) >> 16;
                buf[k + 3] = (b & 0x0000ff00U) >> 8;
                buf[k + 4] = (b & 0x000000ffU);

                // move offset
                k += 5;

                // copy buffer
                memcpy(buf + k, e, b);

                // move offset
                k += b;

                // reset variables
                e = nullptr;
                b = 0;

                break;
            default:
                break;
            }
        }

        // set output length
        // 设置输出数组的长度
        (*outLen) = bufferLength;

        // return pointer of uint8 array
        // 返回数组指针
        return buf;
    }

    /**
     * @brief this function does same thing as bellow, but it accept a callback in argument
     * it will clear memory after callback had been called
     *
     * 跟上面那个功能一样，但是回调函数使用完成之后会自动清理缓存
     *
     * @param elements a vector contains pointers of Element 一个装有Element指针的容器
     *
     * @param callback callback for accept output 接收结果的回调函数
     */
    static void createArrayBuffer(Elements *elements,
                                  createArrayBufferCallback callback)
    {
        // this mark could be set by user, then user could delete output manually
        // but this is not recommended
        // 这个标志可以在回调函数中由用户手动设置，这样在需要更多内存的场景可以立即清除内存
        // 但是不推荐这样做，因为你可能将此标志设为true，但是忘记delete
        bool isBufferDeleted = false;

        // declare a variable for generate function input
        // 声明生成函数所需要传入的长度变量
        uint32_t outLen = 0;

        // generate buffer
        // 生成数组
        uint8_t *buf = ArrayBuffer::createArrayBuffer(elements, &outLen);

        if (!buf || !outLen)
        {
            ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "empty buffer");
        }

        // output pointer and length wheather buffer is empty or not
        // so you should check pointer and length in callback
        // 不论是否生成成功都会返回指针，所以回调函数需要检查指针和长度
        callback(buf, outLen, &isBufferDeleted);

        if (!isBufferDeleted)
        {
            // if user do nothing with buffer, delete it
            // 如果用户没有手动清理缓存就清理掉它
            delete buf;
        }
    }

    /**
     * @brief this function will generate a vector to hold data using a uint8 array buffer provided
     * 这个函数可以根据你提供的二进制数组生成一个vector，里面存储了各种类型的数据
     *
     * @param data uint8 array 二进制数组
     * @param length length of uint8 array 二进制数组的长度
     * @param onlyCopyPointer will NOT copy original data(if type is uint8 array), ONLY copy pointer if this set to true
     * 如果设置为true，则生成的Element如果类型为二进制数组，则 [仅拷贝] 数组指针，而 [不拷贝] 原始数据
     *
     * @return a vector contains pointers of Element 一个装有Element指针的vector容器
     */
    static std::vector<Element *> *decodeArrayBuffer(uint8_t *data,
                                                     uint32_t length,
                                                     bool onlyCopyPointer = false)
    {
        // check pointer and length
        // return null pointer if got invalid inputs
        // 检查指针和输入长度
        // 如果不符合要求则返回空指针
        if (!data || !length)
        {
            ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "empty buffer or zero length when decoding array buffer");
            return nullptr;
        }

        // declare and define container for output
        // 声明定义用于存放输出的容器
        std::vector<Element *> *output = new std::vector<Element *>();

        // offset in source buffer
        // 源数组的偏移量
        uint64_t k = 0;

        // temporary variable for build uint64 element
        // 用于构建64位整数的临时变量
        uint64_t u64 = 0;

        // for calculate length of string and uint8 array
        // 用于存放字符串和二进制数组长度的变量
        uint32_t len;

        uint32_t tmp32 = 0;
        uint64_t tmp64 = 0;
        uint8_t tmpMark = 0;
        float tmpF = 0.0f;
        double tmpD = 0.0f;

        // mark if there is error
        // 指示过程中是否存在错误
        bool error = false;

        // start generating Elements
        // 开始生成元素
        while (k < length)
        {
            if (error)
            {
                // jump out while if error detected
                // 有错误检测到时立即退出
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding");
                break;
            }

            /**
             * @brief the codes following is contrary to createArrayBuffer
             * so there is no more comments
             * will check remainder buffer size before every part
             *
             * 下面的代码与createArrayBuffer的逻辑完全相反
             * 所以就不多翻译了
             * 每次生成新元素之前都会检测剩余buffer的长度
             */
            switch (data[k])
            {
            case MARK_UINT8: // uint8
            case MARK_INT8:  // int8
                if (k + 1 > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint8");
                    error = true;
                    break;
                }

                if (data[k] == MARK_UINT8)
                {
                    output->push_back(new Element((uint8_t)(data[k + 1])));
                }
                else
                {
                    output->push_back(new Element((int8_t)(data[k + 1])));
                }

                k += 2;
                break;
            case MARK_UINT16: // uint16
            case MARK_INT16:  // int16
                if (k + 2 > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint16");
                    error = true;
                    break;
                }
                if (data[k] == MARK_UINT16)
                    output->push_back(new Element((uint16_t)((data[k + 1] << 8) + (data[k + 2]))));
                else
                    output->push_back(new Element((int16_t)((data[k + 1] << 8) + (data[k + 2]))));
                k += 3;
                break;
            case MARK_UINT32: // uint32
            case MARK_INT32:  // int32

                if (k + 4 > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint32");
                    error = true;
                    break;
                }
                tmp32 = (uint32_t)((uint32_t)(data[k + 1] << 24) +
                                   (uint32_t)(data[k + 2] << 16) +
                                   (uint32_t)(data[k + 3] << 8) +
                                   (uint32_t)(data[k + 4]));
                if (data[k] == MARK_UINT32)
                {
                    output->push_back(new Element(tmp32));
                }
                else
                {
                    output->push_back(new Element((int32_t)tmp32));
                }

                tmp32 = 0;

                k += 5;
                break;
            case MARK_FLOAT: // float
                if (k + 4 > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint32");
                    error = true;
                    break;
                }
                ++k;

                memcpy(&tmpF, data + k, 4);

                output->push_back(new Element(tmpF));

                k += 4;
                break;
            case MARK_UINT64: // uint64
            case MARK_INT64:  // int64
                if (k + 8 > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint64");
                    error = true;
                    break;
                }
                tmpMark = data[k];

                // skip mark byte
                // 跳过标志字节位置
                ++k;

                // memcpy(&u64, data + k, 8);

                for (int i = 0; i < 8; ++i)
                {

                    u64 += data[k + i];
                    if (i < 7)
                        u64 <<= 8;
                }

                if (tmpMark == MARK_UINT64)
                {

                    output->push_back(new Element(u64));
                }
                else
                {

                    output->push_back(new Element((int64_t)u64));
                }

                u64 = 0;
                k += 8;
                break;

            case MARK_DOUBLE: // double
                if (k + 8 > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint32");
                    error = true;
                    break;
                }
                ++k;

                memcpy(&tmpD, data + k, 8);

                output->push_back(new Element(tmpD));

                k += 8;
                break;
            case MARK_STRING: // String
            {
                if (k + 4 > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding string length");
                    error = true;
                    break;
                }

                len = (uint32_t)((data[k + 1] << 24) + (data[k + 2] << 16) + (data[k + 3] << 8) + (data[k + 4]));

                k += 5;

                if (!len)
                {
                    // empty string
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "empty string detected");
                    output->push_back(new Element(""));
                    break;
                }

                if (k + len > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding string");
                    error = true;
                    break;
                }

                Element *str = new Element((const char *)data, k, len);

                output->push_back(str);

                k += len;
                break;
            }
            case MARK_BUFFER: // uint8 array
            {
                if (k + 4 > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint8 array length");
                    error = true;
                    break;
                }
                len = (uint32_t)((data[k + 1] << 24) + (data[k + 2] << 16) + (data[k + 3] << 8) + (data[k + 4]));
                k += 5;

                if (k + len > length)
                {
                    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint8 array");
                    error = true;
                    break;
                }

                if (onlyCopyPointer)
                {
                    // directly use constructor of element, but it won't copy data
                    // this is for large binary data transfer and use, like OTA update
                    // 直接使用元素的构造函数，但是不会拷贝数据
                    // 这个方法是为了传输和使用大二进制数组，比如OTA升级
                    output->push_back(new Element(data, len, false, k));
                }
                else
                {
                    // directly use constructor of element to copy data
                    // 直接使用元素的构造函数来拷贝数据

                    output->push_back(new Element(data, len, true, k));
                }

                k += len;
                break;
            }
            default:
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "unknown type when decoding, offset: %llu, data: %u", k, data[k]);
                if (!data[k])
                {
                    ++k;
                }
                break;
            }
        }

        if (error)
        {
            // if error detected will remove all elements and make clean, and return null pointer
            // 如果过程中发生了错误会清除掉所有的元素和容器，返回空指针
            for (
                std::vector<Element *>::iterator it = output->begin();
                it != output->end();
                ++it)
            {
                delete (*it);
            }
            delete output;
            return nullptr;
        }
        return output;
    }

    /**
     * @brief same as bellow
     * 跟上面那个带回调函数的功能一样
     *
     * @param callback callback for accept output 接收结果的回调函数
     * @param data uint8 array 二进制数组
     * @param length length of uint8 array 二进制数组的长度
     */
    static void decodeArrayBuffer(decodeArrayBufferCallback callback,
                                  uint8_t *data,
                                  uint32_t length)
    {
        // check pointer and length
        // 检查指针和长度
        if (!data || !length)
        {
            callback(nullptr);
            return;
        }

        // decode buffer if data valid
        // 如果数据符合要求则开始反序列过程
        std::vector<Element *> *output = ArrayBuffer::decodeArrayBuffer(data, length);

        // call callback
        // 调用回调函数
        callback(output);

        if (output)
        {
            // delete elements
            // 删除对象
            for (uint32_t i = 0; i < output->size(); ++i)
            {
                delete output->at(i);
            }

            // clean
            // 清理
            output->clear();
            std::vector<Element *>().swap(*(output));
            delete output;
        }
    }
};

#endif
