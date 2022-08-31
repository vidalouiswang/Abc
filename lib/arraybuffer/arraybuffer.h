/**
 * @file arraybuffer.h
 * @author Vida Wang (support@vida.wang)
 * @brief Two class in this file implement a simple method to pack a set of data with different types.
    The static function createArrayBuffer will convert a vector contains data into a binary uint8 array,
    which is could be sending through websocket or store it in flash.

    The static function decodeArrayBuffer will extract a list of data with different types from a single
    binary uint8 array, which could be using in everywhere.

    The class Element will hold number, uint8_t, uint16_t, uint32_t uint64_t, string and uint8 array
    will be stored in free store in cpp.

    Two class will help beginners to handle binary data by a more easily way.


    这个文件里的两个类用一个简单的方法帮助新手去处理二进制数据，可以通过相关功能对不同类型的数据进行打包为一个二进制数组。
    静态函数createArrayBuffer可以把一个vector容器转换为一个独立的二进制数组，这个数据就可以用来在网络上传输或者存储了。

    静态函数decodeArrayBuffer可以从一个独立的二进制数组中提取出若干个不同类型的数据，这样可以方便在代码中使用。

    Element这个类用来存储一种数据，它可以是无符号单字节、双字节、四字节、八字节的整数，字符串和另一个二进制数组。
    字符串和二进制数组使用同样的方式存储在C++的自由存储区中。
 * @version 1.0
 * @date 2022-07-22
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once
#include <vector>
#include <Arduino.h>
#include <functional>

#define ARRAY_BUFFER_DEBUG_HEADER "Array Buffer"

/**
 * @brief currently support unsigned number, string and uint8 array.
 * but as you know unsigned and signed types is same in RAM,
 * so you may could change it if you want to use signed numbers.
 *
 * 目前仅支持无符号的整数、字符串和二进制数组
 * 但是因为无符号和有符号在底层存储只是符号位的区别
 * 所以你可以自己改造成可以输出有符号类型的
 */
typedef enum
{
    UINT8,
    UINT16,
    UINT32,
    UINT64,

    // you will got this type when you only want to know if there is a number
    // 当你只想知道是否是一个数字而不关心是几个字节的数字的时候就会返回这个类型
    NUMBER,

    // string and uint8 array will use same way to store
    // 字符串和二进制数组使用同样的方式存储
    STRING,
    U8A,

    // this type will be return if memory allocate failed
    // 当内存分配失败时会返回这个类型
    MEMORY_ALLOCATE_FAILED,
    NONE

} ElementType;

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
    ElementType type = NONE;

    /**
     * @brief hold number
     * 存储数字
     */
    uint64_t number = 0;

    /**
     * @brief hold string or raw uint8 array
     * 存储字符串和二进制数组
     */
    uint8_t *buffer = nullptr;

    /**
     * @brief indicate length of string or uint8 array
     * 表示字符串或者二进制数组的长度
     */
    uint32_t bufferLength = 0;

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
    volatile bool shouldClearBuffer = true;

    /**
     * @brief this function could use same object to change its original data type to another
     * it will clean all buffer(if exists)
     * 这个函数用于给当前对象赋值另一种类型的数据
     * 如果当前对象存储了字符串或者拷贝模式的二进制数组
     * 那么就会清理掉缓存
     *
     * @param type new type 新的类型
     */
    void clearOthers(ElementType type);

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
    bool _setString(const char *data, uint32_t offset = 0, uint32_t length = 0);

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
    bool _copyBuffer(uint8_t *buffer, uint32_t length, uint32_t offset = 0, ElementType type = U8A);

public:
    /**
     * @brief Set the Number object
     * 直接修改当前对象存储的数字
     *
     * @param n number 数字
     * @param forceType manually set type 手动设置类型
     */
    void setNumber(uint64_t n, ElementType forceType = NONE);

    /**
     * @brief this function will indicate current object if has available data
     * 这个函数用来指示当前对象是否存储了有用的数据
     *
     * @return true object stored valid data 对象存储了有效的数据可供读取
     * @return false object hasn't any valid data stored 对象没有存储任何有效数据
     */
    inline bool available() const
    {
        return this->type != NONE && this->type != MEMORY_ALLOCATE_FAILED;
    }

    /**
     * @brief manually copy object by developer
     * 开发者可以使用这两个函数手动从另一个对象或者缓存拷贝数据
     *
     * @param element pointer of another object
     * 另一个对象的指针
     * @return true success 成功
     * @return false failed 失败
     */
    bool copyFrom(Element *element);

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
    bool copyFrom(uint8_t *buffer, uint64_t length);

    /**
     * @brief default constructor, type set to NONE
     * 默认无参构造函数, 类型被设置为NONE
     */
    inline Element() { this->type = NONE; }

    /**
     * @brief copy from another element, this is a special copy constructor
     * using pointer of another object
     * 默认的“拷贝构造函数”， 可以从另一个对象拷贝一模一样的数据
     * 只不过是使用另一个对象的指针作为参数
     * @param e pointer of another object 另一个对象的指针
     */
    inline Element(Element *e)
    {
        this->copyFrom(e);
    }

    /**
     * @brief quickly set number( automatically detect value type)
     * when using vscode as editor, number will show as int
     * when you writing it directly
     *
     * 快速的设置当前对象的数字
     * 因为使用vscode的时候，直接使用构造函数默认会表示为int类型
     *
     * @param n number 设置的数字
     */
    inline Element(int n) { this->setNumber(n); }
    inline Element(long n) { this->setNumber(n); }
    inline Element(long long n) { this->setNumber(n); }

    /**
     * @brief specific constructor
     * 专门类型的构造函数
     *
     * @param a number 数字
     */
    inline Element(uint8_t a) : number(a), type(UINT8) {}
    inline Element(uint16_t b) : number(b), type(UINT16) {}
    inline Element(uint32_t c) : number(c), type(UINT32) {}
    inline Element(uint64_t d) : number(d), type(UINT64) {}

    /**
     * @brief string constructor, always make a copy
     * 字符串的构造函数，会拷贝原始字符串
     *
     * @param data c string or arduino String(object or pointer) c字符串或Arduino字符串(对象或指针)
     */
    inline Element(const char *data) { this->_setString(data); }
    inline Element(String e) { this->_setString(e.c_str()); }
    inline Element(String *e) { this->_setString(e->c_str()); }

    /**
     * @brief string constructor, copy with offset in another buffer
     * 字符串的构造函数，从源数组的指定位置开始拷贝
     *
     * @param buffer source buffer 拷贝源
     * @param offset offset in source 源数组中开始拷贝的偏移量
     * @param length length that string should copy 需要拷贝的字符串的长度
     */
    explicit inline Element(const char *buffer, uint32_t offset, uint32_t length)
    {
        this->_setString(buffer, offset, length);
    }

    /**
     * @brief raw uint8 array constructor, this will copy from original buffer
     * 二进制数组的构造函数，会从原始缓存拷贝一份原始数据
     *
     * @param buffer source uint8 array 源数组
     * @param bufferLength length of uint8 array 数组长度
     * @param offsetInSrc offset in source 源数组开始拷贝的偏移量
     */
    explicit inline Element(uint8_t *buffer, uint32_t bufferLength, uint32_t offsetInSrc = 0)
    {
        if (!this->_copyBuffer(buffer, bufferLength, offsetInSrc))
        {
            this->type = NONE;
            this->buffer = nullptr;
            this->bufferLength = 0;
        }
    }

    /**
     * @brief special constructor using by developer, will NOT copy original data, ONLY copy pointer
     * 特殊的二进制数组构造函数，这个构造函数 [仅拷贝] 指针，[不会] 拷贝原始数据
     *
     * @param dontCopyBuffer only for using this constructor, no matter true of false 只是为了用这个构造, true false都可以
     * @param buffer source uint8 array 源二进制数组
     * @param bufferLength length of uint8 array 二进制数组长度
     */
    inline Element(bool dontCopyBuffer, uint8_t *buffer, uint32_t bufferLength)
        : buffer(buffer),
          bufferLength(bufferLength),
          type(U8A),
          shouldClearBuffer(false)
    {
    }

    /// operators
    /// 重载的运算符，这个非常简单没啥好说的自己看看就懂了

    bool operator=(const char *data)
    {
        return this->_setString(data);
    }

    inline bool operator=(const String data)
    {
        return this->_setString(data.c_str());
    }

    inline bool operator=(const String *data)
    {
        return this->_setString(data->c_str());
    }

    inline bool operator=(int data)
    {
        this->setNumber(data);
        return true;
    }

    inline bool operator=(uint8_t data)
    {
        this->setNumber(data, UINT8);
        return true;
    }

    inline bool operator=(uint16_t data)
    {
        this->setNumber(data, UINT16);
        return true;
    }

    inline bool operator=(uint32_t data)
    {
        this->setNumber(data, UINT32);
        return true;
    }

    inline bool operator=(uint64_t data)
    {
        this->setNumber(data, UINT64);
        return true;
    }

    inline bool operator=(Element data)
    {
        return this->copyFrom(&data);
    }

    inline bool operator=(Element *data)
    {
        return this->copyFrom(data);
    }

    inline bool operator()(uint8_t *buffer, uint64_t length)
    {
        return this->copyFrom(buffer, length);
    }

    inline bool operator==(const Element *obj) const { return this->equalsTo(obj); }
    inline bool operator==(const Element &obj) const { return obj.equalsTo(this); }
    inline bool operator==(const int &rvalue) const { return this->number == rvalue; }
    inline bool operator==(const uint64_t &rvalue) const { return this->number == rvalue; }
    bool equalsTo(const Element *obj) const;

    static bool compareElements(const Element *x, const Element *y, bool lessThan);

    inline bool operator<(const Element &obj) const { return Element::compareElements(this, &obj, true); }
    inline bool operator<(const int &rvalue) const { return this->number < rvalue; }
    inline bool operator<(const uint64_t &rvalue) const { return this->number < rvalue; }

    inline bool operator>(const Element &obj) const { return Element::compareElements(this, &obj, false); }
    inline bool operator>(const int &rvalue) const { return this->number > rvalue; }
    inline bool operator>(const uint64_t &rvalue) const { return this->number > rvalue; }

    inline bool operator<=(const Element &obj) const { return (this->equalsTo(&obj) || Element::compareElements(this, &obj, true)); }
    inline bool operator>=(const Element &obj) const { return (this->equalsTo(&obj) || Element::compareElements(this, &obj, false)); }

    inline bool operator<=(const int &rvalue) const { return this->number <= rvalue; }
    inline bool operator<=(const uint64_t &rvalue) const { return this->number <= rvalue; }

    inline bool operator>=(const int &rvalue) const { return this->number >= rvalue; }
    inline bool operator>=(const uint64_t &rvalue) const { return this->number >= rvalue; }

    // basic math operators
    // 基本数学符号重载

    // ADD
    inline uint64_t operator+(const int &rvalue) const
    {
        return this->number + rvalue;
    }
    inline uint64_t operator+(const Element &rvalue) const
    {
        return this->number + rvalue.getNumber();
    }
    inline String operator+(const String &rvalue) const
    {
        return this->getString() + rvalue;
    }

    // SUB
    inline uint64_t operator-(const int &rvalue) const
    {
        return this->number - rvalue;
    }
    inline uint64_t operator-(const Element &rvalue) const
    {
        return this->number - rvalue.getNumber();
    }
    inline String operator-(const String &rvalue)
    {
        if (this->type == STRING)
        {
            String tmp = this->getString();
            tmp.replace(rvalue, "");
            return tmp;
        }
        return "";
    }

    // MUL
    inline uint64_t operator*(const int &rvalue) const
    {
        return this->number * rvalue;
    }
    inline uint64_t operator*(const Element &rvalue) const
    {
        return this->number * rvalue.getNumber();
    }

    // DIV
    inline uint64_t operator/(const int &rvalue) const
    {
        return this->number / rvalue;
    }
    inline uint64_t operator/(const Element &rvalue) const
    {
        return this->number / rvalue.getNumber();
    }

    // +=
    inline Element &operator+=(const int &rvalue)
    {
        this->number += rvalue;
        return (*this);
    }
    inline Element &operator+=(const Element &rvalue)
    {
        this->number += rvalue.getNumber();
        return (*this);
    }
    inline Element &operator+=(const String &rvalue)
    {
        if (this->type == STRING)
            this->setString(this->getString() + rvalue);
        return (*this);
    }
    inline Element &operator+=(const char *rvalue)
    {
        if (this->type == STRING)
            this->setString(this->getString() + rvalue);
        return (*this);
    }

    // -=
    inline Element &operator-=(const int &rvalue)
    {
        this->number -= rvalue;
        return (*this);
    }
    inline Element &operator-=(const Element &rvalue)
    {
        this->number -= rvalue.getNumber();
        return (*this);
    }
    inline Element &operator-=(const String &rvalue)
    {
        if (this->type == STRING)
        {
            String tmp = this->getString();
            tmp.replace(rvalue, "");
            this->setString(tmp);
        }
        return (*this);
    }

    // *=
    inline Element &operator*=(const int &rvalue)
    {
        this->number *= rvalue;
        return (*this);
    }
    inline Element &operator*=(const Element &rvalue)
    {
        this->number *= rvalue.getNumber();
        return (*this);
    }

    // /=
    inline Element &operator/=(const int &rvalue)
    {
        this->number /= rvalue;
        return (*this);
    }
    inline Element &operator/=(const Element &rvalue)
    {
        this->number /= rvalue.getNumber();
        return (*this);
    }

    // ++
    inline Element &operator++()
    {
        if (this->getType(true) == NUMBER)
            ++(this->number);
        return (*this);
    }
    inline const Element operator++(int)
    {
        if (this->getType(true) != NUMBER)
            return (*this);
        Element tmp = *this;
        ++(*this);
        return tmp;
    }

    // --
    inline Element &operator--()
    {
        if (this->getType(true) == NUMBER)
            --(this->number);
        else if (this->type == STRING)
            this->setString(this->getString().substring(0, this->bufferLength - 2));
        return (*this);
    }
    inline const Element operator--(int)
    {
        if (this->getType(true) != NUMBER)
            return (*this);
        Element tmp = *this;
        if (this->getType(true) == NUMBER)
            --(this->number);
        else if (this->type == STRING)
            this->setString(this->getString().substring(1, this->bufferLength - 1));
        return tmp;
    }

    // %
    inline uint64_t operator%(const int &rvalue) const
    {
        return this->number % rvalue;
    }
    inline uint64_t operator%(const Element &rvalue) const
    {
        return this->number % rvalue.getNumber();
    }
    inline Element &operator%=(const int &rvalue)
    {
        this->number %= rvalue;
        return (*this);
    }
    inline Element &operator%=(const Element &rvalue)
    {
        this->number %= rvalue.getNumber();
        return (*this);
    }

    // <<
    inline uint64_t operator<<(const int &rvalue) const
    {
        return this->number << rvalue;
    }
    inline uint64_t operator<<(const Element &rvalue) const
    {
        return this->number << rvalue.getNumber();
    }
    inline Element &operator<<=(const int &rvalue)
    {
        this->number <<= rvalue;
        return (*this);
    }
    inline Element &operator<<=(const Element &rvalue)
    {
        this->number <<= rvalue.getNumber();
        return (*this);
    }

    // >>
    inline int operator>>(const int &rvalue) const
    {
        if (this->getType(true) != NUMBER)
            return -2;
        return this->number >> rvalue;
    }
    inline int operator>>(const Element &rvalue) const
    {
        if (this->getType(true) != NUMBER)
            return -2;
        return this->number >> rvalue.getNumber();
    }
    inline Element &operator>>=(const int &rvalue)
    {
        this->number >>= rvalue;
        return (*this);
    }
    inline Element &operator>>=(const Element &rvalue)
    {
        this->number >>= rvalue.getNumber();
        return (*this);
    }

    // &
    inline uint64_t operator&(const uint64_t &rvalue) const
    {
        return this->number & rvalue;
    }
    inline Element &operator&=(const int &rvalue)
    {
        this->number &= rvalue;
        return (*this);
    }
    inline Element &operator&=(const Element &rvalue)
    {
        this->number &= rvalue.getNumber();
        return (*this);
    }

    // |
    inline uint64_t operator|(const uint64_t &rvalue) const
    {
        return this->number | rvalue;
    }
    inline Element &operator|=(const int &rvalue)
    {
        this->number |= rvalue;
        return (*this);
    }
    inline Element &operator|=(const Element &rvalue)
    {
        this->number |= rvalue.getNumber();
        return (*this);
    }

    // !
    inline bool operator!() const
    {
        return (bool)(!(this->number));
    }

    // ^
    inline uint64_t operator^(const uint64_t &rvalue) const
    {
        return this->number ^ rvalue;
    }
    inline Element &operator^=(const int &rvalue)
    {
        this->number ^= rvalue;
        return (*this);
    }
    inline Element &operator^=(const Element &rvalue)
    {
        this->number ^= rvalue.getNumber();
        return (*this);
    }

    // ~
    inline uint64_t operator~() const
    {
        return ~(this->number);
    }

    // []
    inline uint8_t operator[](const int &rvalue)
    {
        if (this->bufferLength)
            if (rvalue < this->bufferLength - 1)
                return this->buffer[rvalue];
        return -2;
    }
    inline uint8_t operator[](const Element &rvalue)
    {
        if (rvalue.getType(true) != NUMBER)
            return -2;
        if (this->bufferLength)
            if (rvalue.getNumber() < this->bufferLength - 1)
                return this->buffer[rvalue.getNumber()];
        return -2;
    }

    // &&
    inline bool operator&&(const int &rvalue) const
    {
        return (bool)(this->number & rvalue);
    }
    inline bool operator&&(const Element &rvalue) const
    {
        return (bool)(this->number & rvalue.getNumber());
    }

    // ||
    inline bool operator||(const int &rvalue) const
    {
        return (bool)(this->number | rvalue);
    }
    inline bool operator||(const Element &rvalue) const
    {
        return (bool)(this->number | rvalue.getNumber());
    }

    /// operators

    inline const char *c_str() const
    {
        if (this->type == STRING)
            return (const char *)(this->buffer);
        return "";
    }

    inline int indexOf(const String &target) const
    {
        if (this->type == STRING)
            return this->getString().indexOf(target);
        return -2;
    }
    inline int indexOf(const char &target) const
    {
        if (this->type == STRING)
            for (uint32_t i = 0; i < this->bufferLength; ++i)
                if (target == this->buffer[i])
                    return i;

        return -2;
    }

    inline int lastIndexOf(const String &target) const
    {
        if (this->type == STRING)
            return this->getString().lastIndexOf(target);
        return -2;
    }
    inline int lastIndexOf(const char &target) const
    {
        if (this->type == STRING)
            for (uint32_t i = this->bufferLength - 1; i >= 0; --i)
                if (target == this->buffer[i])
                    return i;
        return -2;
    }

    /**
     * @brief deconstructor, it will clean buffer automatically
     * 析构函数， 它会调用清理缓存的函数
     */
    ~Element();

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
    ElementType getType(bool isNumber = false) const;

    /**
     * @brief get number
     * 获取存储的数字
     *
     * this part could be written as
     *   template<class T>
     *   inline T getNumber() const { return (T)this->number; }
     *
     *   using:
     *      obj->getNumber<uint8_t>();
     *
     *   but I like to write it in this way
     *
     *   这部分其实可以用模板
     *   想上面这样写和用，但是我喜欢更直观的方式写这部分代码
     *
     * @return stored number
     */
    inline uint8_t getUint8() const { return (uint8_t)this->number; }
    inline uint16_t getUint16() const { return (uint16_t)this->number; }
    inline uint32_t getUint32() const { return (uint32_t)this->number; }
    inline uint64_t getUint64() const { return this->number; }

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
        return (this->type == STRING) ? (this->buffer ? (String((const char *)this->buffer)) : (String("")))
                                      : ((this->type == U8A) ? (this->getHex()) : (String("")));
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
        if (this->type == U8A)
        {
            if (outLen)
                (*outLen) = this->bufferLength;
            return (uint8_t *)(this->buffer);
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
    bool convertHexStringIntoUint8Array();

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
            (*outLen) = this->bufferLength;

        return (uint8_t *)(this->buffer);
    }

    /**
     * @brief get length of uint8 array if current object stroed uint8 array
     * will return 0 if object stored other type data
     *
     * 获取二进制数组的长度，当且仅当存储的数据类型是二进制数组的时候才返回长度，否则返回0
     *
     * @return length of uint8 array 存储的数组的长度
     */
    inline uint32_t getU8aLen() const { return this->type == U8A ? this->bufferLength : 0; }

    /**
     * @brief more simply way to get number, because of all types of number stored as uint64_t
     * and they are unsigned
     *
     * 更方便的方式去获取存储的数字，因为都是无符号数字，都是用8字节存储的，所以怎么用取决于你
     *
     * @return stored number 存储的数字
     */
    inline uint64_t getNumber() const
    {
        return this->number;
    }

    /**
     * @brief directly set a string, attention attached
     * 直接设置/更改当前对象存储一个字符串, 请阅读注意事项
     *
     * @attention THIS FUNCTION WILL CLEAN BUFFER AUTOMATICALLY
     * 这个函数会清理缓存(如果对象原来存储了二进制数组或者字符串，那就被清理了)
     *
     * @param e new string 新的字符串
     */
    inline void setString(String e)
    {
        this->_setString(e.c_str());
    }

    /**
     * @brief directly set a string, attention attached
     * 直接设置/更改当前对象存储一个字符串, 请阅读注意事项
     *
     * @attention THIS FUNCTION WILL CLEAN BUFFER AUTOMATICALLY
     * 这个函数会清理缓存(如果对象原来存储了二进制数组或者字符串，那就被清理了)
     *
     * @param e new string 新的字符串
     */
    inline void setString(String *e)
    {
        this->_setString(e->c_str());
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
        this->clearOthers(U8A);
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
    String getHex(bool lowerCase = true) const;

    /**
     * @brief get length of raw buffer whether object stored string or uint8 array
     * 直接获取原始buffer的长度
     *
     * @return length of raw uint8 array 缓存长度
     */
    uint64_t getRawBufferLength() const
    {
        return this->bufferLength;
    }

    /**
     * @brief this function is for createArrayBuffer using
     * 这个函数是给createArrayBuffer用的
     *
     * @param includeHeader include length of header 是否带有头部长度
     *
     * @return length 长度
     */
    uint64_t getBufferLength(bool includeHeader = false) const;

    /**
     * @brief
     * clear buffer
     * 清理缓存的函数
     */
    void clearBuffer();
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
                                      uint64_t *outLen);

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
                                  createArrayBufferCallback callback);

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
                                                     uint64_t length,
                                                     bool onlyCopyPointer = false);

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
                                  uint64_t length);
};