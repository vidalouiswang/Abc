#include "arraybuffer.h"

uint8_t *ArrayBuffer::createArrayBuffer(std::vector<Element *> *elements,
                                        uint64_t *outLen)
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
        case UINT8:
            // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "UINT8");
            bufferLength += 2;
            break;
        case UINT16:
            // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "UINT16");
            bufferLength += 3;
            break;
        case UINT32:
            // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "UINT32");
            bufferLength += 5;
            break;
        case UINT64:
            // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "UINT64");
            bufferLength += 9;
            break;
        case STRING:
            // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "STRING");
            bufferLength += 5;
            bufferLength += (*it)->getRawBufferLength() - 1;
            break;
        case U8A:
            // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "U8A");
            bufferLength += 5;
            bufferLength += (*it)->getU8aLen();
            break;
        case NONE:
            ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "NONE type");
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

    // start to fill data
    // 从这里开始装填数据
    // 其实用一个字节来表示类型太浪费了，但是太方便了
    for (; it != end; ++it)
    {
        // mark will took 1 byte
        // 标志位会占用一个字节
        switch ((*it)->getType())
        {
        case UINT8:
            // 2 bytes total
            // 一共2字节
            // set mark
            // 设置标志位
            // 下面一样的就不翻译了
            buf[k] = 0x80;

            // set data
            buf[k + 1] = (*it)->getUint8();

            // move offset
            k += 2;
            break;
        case UINT16:
            // 3 bytes total
            // set mark
            buf[k] = 0x81;

            // get number
            a = (*it)->getUint16();

            // fill data
            buf[k + 1] = (a & 0xff00U) >> 8;
            buf[k + 2] = (a & 0x00ffU);

            // move offset
            k += 3;

            a = 0;
            break;
        case UINT32:
            // 5 bytes total
            // set mark
            buf[k] = 0x82;

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
        case UINT64:
            // 9 bytes total
            // set mark
            buf[k] = 0x83;

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
        case STRING:
            // 5 + length of string total
            {
                // get length of string
                // 获取字符串的长度
                b = (*it)->getRawBufferLength() - 1;

                // set mark
                buf[k] = 0x86;

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
        case U8A:
            // 5 + length of buffer total

            // get buffer pointer and length of buffer to "b"
            e = (*it)->getUint8Array(&b);

            // set mark
            buf[k] = 0x85;

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

void ArrayBuffer::createArrayBuffer(std::vector<Element *> *elements,
                                    std::function<void(uint8_t *output, uint64_t length, bool *isBufferDeleted)> callback)
{

    // this mark could be set by user, then user could delete output manually
    // but this is not recommended
    // 这个标志可以在回调函数中由用户手动设置，这样在需要更多内存的场景可以立即清除内存
    // 但是不推荐这样做，因为你可能将此标志设为true，但是忘记delete
    bool isBufferDeleted = false;

    // declare a variable for generate function input
    // 声明生成函数所需要传入的长度变量
    uint64_t outLen = 0;

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

std::vector<Element *> *ArrayBuffer::decodeArrayBuffer(uint8_t *data,
                                                       uint64_t length,
                                                       bool onlyCopyPointer)
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
        case 0x80: // uint8
            if (k + 1 > length)
            {
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint8");
                error = true;
                break;
            }
            output->push_back(new Element(data[k + 1]));
            k += 2;
            break;
        case 0x81: // uint16
            if (k + 2 > length)
            {
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint16");
                error = true;
                break;
            }
            output->push_back(new Element((uint16_t)((data[k + 1] << 8) + (data[k + 2]))));
            k += 3;
            break;
        case 0x82: // uint32
            if (k + 4 > length)
            {
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint32");
                error = true;
                break;
            }
            output->push_back(
                new Element(
                    (uint32_t)((uint32_t)(data[k + 1] << 24) +
                               (uint32_t)(data[k + 2] << 16) +
                               (uint32_t)(data[k + 3] << 8) +
                               (uint32_t)(data[k + 4]))));
            k += 5;
            break;
        case 0x83: // uint64
            if (k + 8 > length)
            {
                ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "error when decoding uint64");
                error = true;
                break;
            }

            // skip mark byte
            // 跳过标志字节位置
            ++k;

            for (int i = 0; i < 8; i++)
            {
                u64 += data[k + i];
                if (i < 7)
                    u64 <<= 8;
            }

            output->push_back(new Element(u64));
            u64 = 0;
            k += 8;
            break;
        case 0x86: // String
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

            output->push_back(new Element((const char *)data, k, len));

            k += len;
            break;
        }
        case 0x85: // uint8 array
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
                output->push_back(new Element(true, data + k, len));
            }
            else
            {
                // directly use constructor of element to copy data
                // 直接使用元素的构造函数来拷贝数据
                output->push_back(new Element(data, len, k));
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

void ArrayBuffer::decodeArrayBuffer(std::function<void(std::vector<Element *> *output)> callback,
                                    uint8_t *data,
                                    uint64_t length)
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

bool Element::equalsTo(const Element *obj) const
{
    if (obj->getType() != this->type)
    {
        // return false if two elements have different type
        // 如果两个对象所存储的数据类型不一样直接返回false
        ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "type unequaled, a type: %d, b type: %d", obj->getType(), this->type);
        return false;
    }

    if (this->type == U8A)
    {
        // return false if length is unequaled of two elements when they stored uint8 array
        // 如果两个对象都存储了二进制数组但是长度不一样直接返回false
        if (obj->getU8aLen() != this->bufferLength)
        {
            return false;
        }
        else
        {
            // or, compare every single byte
            // 否则遍历比较每个字节
            uint8_t *objU8A = obj->getUint8Array();
            for (uint32_t i = 0; i < this->bufferLength; ++i)
            {
                if (objU8A[i] != this->buffer[i])
                {
                    return false;
                }
            }
            return true;
        }
    }
    else
    {
        switch (this->type)
        {
        // compare number if they both stored numner
        // 数字直接进行比较
        case UINT8:
            return obj->getUint8() == (uint8_t)this->number;
        case UINT16:
            return obj->getUint16() == (uint16_t)this->number;
        case UINT32:
            return obj->getUint32() == (uint32_t)this->number;
        case UINT64:
            return obj->getUint64() == this->number;
        case STRING:
            if (this->bufferLength &&
                this->buffer &&
                obj->getRawBufferLength() &&
                obj->getRawBuffer())
            {
                // compare using override operator String class directly
                // 直接使用String类重载的逻辑等比较
                return obj->getString() == String((const char *)this->buffer);
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool Element::compareElements(const Element *x, const Element *y, bool lessThan)
{
    // a static function of class to compare two elements
    // 静态函数比较两个对象的大小
    if (!x || !y)
        return false;
    if (!x->available() || !y->available())
        return false;

    ElementType typeX = x->getType(true);
    ElementType typeY = y->getType(true);

    if (typeX != typeY)
        return false;

    if (typeX == NUMBER)
    {
        if (lessThan)
            return x->getNumber() < y->getNumber();
        else
            return x->getNumber() > y->getNumber();
    }
    else if (typeX == STRING)
    {
        if (lessThan)
            return x->getString() < y->getString();
        else
            return x->getString() > y->getString();
    }
    else
    {
        if (x->getU8aLen() != y->getU8aLen())
        {
            return false;
        }
        else
        {
            // zero set to false
            // 返回值0被设置为false
            int result = strcmp((const char *)x->getUint8Array(), (const char *)y->getUint8Array());
            if (lessThan)
            {
                if (result < 0)
                    return true;
                else
                    return false;
            }
            else
            {
                if (result > 0)
                    return true;
                else
                    return false;
            }
        }
    }
}

void Element::setNumber(uint64_t n, ElementType forceType)
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
    if (n < 256)
    {
        clearOthers(UINT8);
    }
    else if (n >= 256 && n < 65536)
    {
        clearOthers(UINT16);
    }
    else if (n >= 65536 && n < 4294967296)
    {
        clearOthers(UINT32);
    }
    else
    {
        clearOthers(UINT64);
    }
    if (forceType != NONE)
    {
        this->type = forceType;
    }
    this->number = n;
}

bool Element::_setString(const char *data, uint32_t offset, uint32_t length)
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
            this->clearOthers(STRING);

            // copy original data if length is non zero
            // 如果不是空字符串就拷贝数据
            return this->_copyBuffer((uint8_t *)data, length, offset, STRING);
        }
    }
    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "type set to NONE");
    this->type = NONE;
    return false;
}

bool Element::convertHexStringIntoUint8Array()
{
    // check type and length 检查类型和长度
    if (this->type != STRING || (!this->buffer) || (!this->bufferLength))
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
        sscanf((const char *)(this->buffer + i), "%02x", &tmp);
        this->buffer[k] = (uint8_t)tmp;
        ++k;
    }

    // modify type 修改类型
    this->type = U8A;

    // modify length 修改长度
    this->bufferLength = len;

    return true;
}

Element::~Element()
{
    // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "object de, address: %x", this);

    // clear buffer if buffer is copied from other places
    // 如果buffer是从其他位置拷贝的就清除buffer
    if (this->shouldClearBuffer)
    {
        this->clearBuffer();
    }
}

void Element::clearBuffer()
{
    // ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "buffer length: %lu", this->bufferLength);
    if (this->buffer)
    {

        // clear length 归零长度
        this->bufferLength = 0;

        // clear buffer 清除buffer
        delete this->buffer;

        // reset pointer 重置指针
        this->buffer = nullptr;
    }
}

String Element::getHex(bool lowerCase) const
{
    if (this->type != U8A)
    {
        if (this->type == STRING)
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
    uint32_t length = (this->bufferLength * 2) + 1;

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
    for (uint32_t i = 0; i < this->bufferLength; i++)
    {
        sprintf(buf + (i * 2), hexCase, this->buffer[i]);
    }

    return String(buf);
}

void Element::clearOthers(ElementType type)
{
    switch (this->type)
    {
        // number will be set to 0
        // 数字直接设置为0
    case UINT8:
    case UINT16:
    case UINT32:
    case UINT64:
        this->number = 0;
        break;

        // uint8 array and string will call clear function
        // 字节数组和字符串会调用清理缓存函数
    case U8A:
    case STRING:
        if (this->bufferLength && this->buffer)
        {
            this->clearBuffer();
        }
        break;
    }

    // set current type to given type
    // 设定当前类型为给定的类型
    this->type = type;
}

uint64_t Element::getBufferLength(bool includeHeader) const
{
    switch (this->type)
    {
    case UINT8:
        return includeHeader ? 2 : 1;
    case UINT16:
        return includeHeader ? 3 : 1;
    case UINT32:
        return includeHeader ? 5 : 1;
    case UINT64:
        return includeHeader ? 9 : 1;
    case STRING:
        return includeHeader ? this->bufferLength + 5 : this->bufferLength + 1;
    case U8A:
        return includeHeader ? this->bufferLength + 5 : this->bufferLength;
    }
    return 0;
}

ElementType Element::getType(bool isNumber) const
{

    return isNumber ? ((this->type == UINT8 ||
                        this->type == UINT16 ||
                        this->type == UINT32 ||
                        this->type == UINT64)
                           ? NUMBER
                           : this->type)
                    : (this->type);
}

bool Element::_copyBuffer(uint8_t *buffer, uint32_t length, uint32_t offset, ElementType type)
{
    // allocate buffer
    // 分配内存
    length = type == STRING ? length + 1 : length;
    this->buffer = new (std::nothrow) uint8_t[length];

    // check memory allocation
    // 检测内存分配
    if (!this->buffer)
    {
        this->type = MEMORY_ALLOCATE_FAILED;
        this->bufferLength = 0;
        return false;
    }
    memset(this->buffer, 0, length);

    // copy from origin
    // 从原始位置拷贝数据
    memcpy(this->buffer, buffer + offset, length);
    if (type == STRING)
    {
        this->buffer[length - 1] = 0;
    }
    this->type = type;
    this->bufferLength = length;
    this->shouldClearBuffer = true;
    return true;
}

bool Element::copyFrom(uint8_t *buffer, uint64_t length)
{
    this->clearBuffer();
    return this->_copyBuffer(buffer, length, 0);
}

bool Element::copyFrom(Element *e)
{
    if (!e)
    {
        ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "copy target is nullptr");
        return false;
    }

    if (e->getType() == NONE)
    {
        ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "copy target is NONE type");
        return false;
    }
    ElementType type = e->getType();
    switch (type)
    {
    case UINT8:
    case UINT16:
    case UINT32:
    case UINT64:
        this->type = type;
        this->number = e->getNumber();
        break;
    case STRING:
        return this->_setString((const char *)e->getRawBuffer(), 0, e->getRawBufferLength() - 1);
    case U8A:
        ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "copy from another element u8a");
        return this->copyFrom(e->getUint8Array(), e->getU8aLen());
    default:
        return false;
        break;
    }
    ESP_LOGD(ARRAY_BUFFER_DEBUG_HEADER, "copy finished");
    return true;
}