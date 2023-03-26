/**
 * @file mydb.h
 * @author Vida Wang (support@vida.wang)
 * @brief This file implements a key-value typed RAM database,
 * designed to help beginners access data in flash using binary format.
 *
 * 这个文件实现了一个键值对型内存数据库，旨在帮助新手使用二进制格式在flash方便的存取数据。
 *
 * @version 1.0.0
 * @date 2022-08-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MYDB_H_
#define MYDB_H_

#include <myfs.h>
#include <arraybuffer.hpp>
#include <vector>

#define MYDB_DEBUG_HEADER "mydb"

class Unit
{
private:
public:
    // key
    // 键
    Element *key = nullptr;

    // value
    // 值
    Element *value = nullptr;

    Unit() {}

    /**
     * @brief default unit constructor
     * 默认unit类构造函数
     *
     * @param key key 键
     * @param value value 值
     */
    inline Unit(Element *key, Element *value = nullptr) : key(key), value(value) {}

    inline ~Unit()
    {
        // remove object
        // 删除对象
        if (key)
            delete key;
        if (value)
            delete value;
    }

    // clear value
    // 清除值
    inline void removeValue()
    {
        if (this->value)
        {
            delete this->value;
            this->value = new Element();
        }
    }
};

typedef std::vector<Unit *> Units;

/**
 * @brief because of all data stored in RAM,
 * so directly use sequential search
 * time complexity is O(n)
 *
 * 因为数据是存储在内存中的，所以直接使用顺序查找
 * 时间复杂度O(n)
 *
 */
class MyDB
{
private:
    // database file name
    // 数据库文件名称
    String name = "";

    // database container
    // 数据库内存容器
    std::vector<Unit *> *container = nullptr;

    /**
     * @brief find index from container
     * 从容器中找到指定key的索引
     *
     * @param key key 键
     * @return int64_t index of unit, unit 的索引
     */
    int64_t findIndex(Element *key);

    /**
     * @brief read data file
     * 读取文件
     *
     * @param mainFile read main file or backup file
     * 读取主文件还是备份文件
     * @return std::vector<Element *>* decoded data 已解码的数据
     */
    std::vector<Element *> *readFile(bool mainFile = true);

    /**
     * @brief clear buffer
     * 清除缓存
     *
     * @param list target 被清除的对象
     */
    void removeBuffer(std::vector<Element *> *list);

    /**
     * @brief build container, convert Element* to Unit*
     * 构建容器，从Element* 转换为Unit*
     *
     * @param list deocded data 已解码的数据
     */
    void buildContainer(std::vector<Element *> *list);

    /**
     * @brief create backup file
     * 创建备份文件
     *
     * @return true success 成功
     * @return false failed 失败
     */
    bool makeBackup();

public:
    /**
     * @brief indicate database loaded or not
     *
     * 指示数据库是否已被加载
     *
     */
    bool loaded = false;

    MyDB();

    /**
     * @brief load data from flash and build container, attention attached
     * 从flash加载数据然后构建容器, 请阅读注意事项
     *
     * @attention must call this function before using other functions
     * 在调用其他功能之前必须先调用此函数
     *
     * @return true success 成功
     * @return false failed 失败
     */
    bool begin();

    /**
     * @brief serialize database to uint8 array
     * 序列化数据库为uint8数组
     *
     * @note web could use this to backup databse
     * 前端可以使用此函数备份数据库
     *
     * @param buffer pointer to buffer pointer 指向缓存的二级指针
     * @param outLen length of output buffer 输出数据的长度
     */
    void dump(uint8_t **buffer, uint32_t *outLen);

    /**
     * @brief create a backup file and flush all data into flash, attention attached
     * 创建备份文件然后将所有数据写入flash, 请阅读注意事项
     *
     * @attention if value of a unit is unavailable(!target->value->available()), it WILL NOT be flush into flash
     * 如果一个unit的value不可用(!target->value->available())，那么它 [不会] 被写入flash
     *
     * @return true success 成功
     * @return false failed 失败
     */
    bool flush();

    /**
     * @brief get number of data stored in database
     * 获取数据库中存储数据的数量
     *
     * @param selectAll select all units, include unavailable
     * 选择所有units，包括不可用的
     *
     * @return uint32_t number 数量
     */
    inline uint32_t count(bool selectAll = true)
    {
        return this->container ? (
                                     selectAll ? (
                                                     this->container->size())
                                               : (
                                                     std::count_if(
                                                         this->container->begin(), // vector start
                                                         this->container->end(),   // vector end
                                                         [](Unit *unit)
                                                         {
                                                             return unit->value->available();
                                                         })))
                               : (0);
    }

    /**
     * @brief default constructor to create database
     * 默认创建数据库的构造函数
     *
     * @param DBName name of database 数据库名称
     */
    inline MyDB(const char *DBName) : name(DBName) {}

    /**
     * @brief get container for other purpose, attention attached
     * 获取数据库容器用于其他用途，请阅读注意事项
     *
     * @attention when database haven't been loaded, it will return
     * null pointer
     *
     * 当数据库未加载或已经被卸载时，会返回空指针
     *
     * @return std::vector<Unit *>* container of current database
     * 当前数据库容器
     */
    inline std::vector<Unit *> *list() const
    {
        return this->container;
    }

    /**
     * @brief use factor way to get a pointer of Element, attention attached
     * 使用仿函数方式获取Element类对象的指针，请阅读注意事项
     *
     * @attention when key DOES NOT exists, it still return a pointer of Element
     * but this object of Element stored nothing internal
     * it is unavailable (target->available() == false)
     *
     * @attention when a empty string provided or database haven't been loaded
     * will return nullptr
     *
     * 当提供了一个空字符串或者数据库尚未加载时，会返回空指针
     *
     * 当检索的项不存在时，仍然返回一个Element对象指针，但是这个对象没有任何数据存储在内部
     * 它是不可用的 (target->available() == false)
     *
     * @param key key in c string format, c字符串键
     * @return Element* pointer of value related to given key
     * 与给定键对应的value的指针
     */
    Element *operator()(const char *key);

    /**
     * @brief use factor way to get a pointer of Element, attention attached
     * 使用仿函数方式获取Element类对象的指针，请阅读注意事项
     *
     * @attention when key DOES NOT exists, it still return a pointer of Element
     * but this object of Element stored nothing internal
     * it is unavailable (target->available() == false)
     *
     * 当检索的项不存在时，仍然返回一个Element对象指针，但是这个对象没有任何数据存储在内部
     * 它是不可用的 (target->available() == false)
     *
     * @attention when a empty string provided, will return nullptr
     * 当提供了一个空字符串时，会返回空指针
     *
     * @param key arduino String class object key, arduino String类对象键
     * @return Element* pointer of value related to given key
     * 与给定键对应的value的指针
     */
    inline Element *operator()(String key)
    {
        return this->operator()(key.c_str());
    }

    /**
     * @brief use factor way to get a pointer of Element, attention attached
     * 使用仿函数方式获取Element类对象的指针，请阅读注意事项
     *
     * @attention when key DOES NOT exists, it still return a pointer of Element
     * but this object of Element stored nothing internal
     * it is unavailable (target->available() == false)
     *
     * 当检索的项不存在时，仍然返回一个Element对象指针，但是这个对象没有任何数据存储在内部
     * 它是不可用的 (target->available() == false)
     *
     * @attention when a empty string provided, will return nullptr
     * 当提供了一个空字符串时，会返回空指针
     *
     * @param key pointer of arduino String class object key, arduino String类对象指针键
     * @return Element* pointer of value related to given key
     * 与给定键对应的value的指针
     */
    inline Element *operator()(String *key)
    {
        return this->operator()(key->c_str());
    }

    /**
     * @brief use factor way to get a pointer of Element, attention attached
     * 使用仿函数方式获取Element类对象的指针，请阅读注意事项
     *
     * @attention when key DOES NOT exists, it still return a pointer of Element
     * but this object of Element stored nothing internal
     * it is unavailable (target->available() == false)
     *
     * 当检索的项不存在时，仍然返回一个Element对象指针，但是这个对象没有任何数据存储在内部
     * 它是不可用的 (target->available() == false)
     *
     * @attention when a null pointer provided
     * or the object that pointer pointed is unavailable
     * will return nullptr
     * 当提供了一个空指针或者指针所指向的对象不可用时，会返回空指针
     *
     * @param key pointer of Element class object key, Element类对象指针键
     * @return Element* pointer of value related to given key
     * 与给定键对应的value的指针
     */
    Element *operator()(Element *key);

    /**
     * @brief use factor way to get a pointer of Element, attention attached
     * 使用仿函数方式获取Element类对象的指针，请阅读注意事项
     *
     * @attention when key DOES NOT exists, it still return a pointer of Element
     * but this object of Element stored nothing internal
     * it is unavailable (target->available() == false)
     *
     * 当检索的项不存在时，仍然返回一个Element对象指针，但是这个对象没有任何数据存储在内部
     * 它是不可用的 (target->available() == false)
     *
     * @attention when object that pointer pointed is unavailable
     * will return nullptr
     * 当提供的对象不可用时，会返回空指针
     *
     * @param key Element class object key, Element类对象键
     * @return Element* pointer of value related to given key
     * 与给定键对应的value的指针
     */
    Element *operator()(Element key)
    {
        return this->operator()(&key);
    }

    /**
     * @brief unload database and clean buffer
     *
     * 卸载数据库然后清理缓存
     *
     */
    void unload();

    /**
     * @brief unload database and delete
     * file ( main file and backup file)
     *
     * 卸载数据库然后删除文件 (主文件与备份文件)
     *
     * @param currentDataBaseName name of current database
     * for confirmation
     *
     * 当前数据库的名称，用于确认操作
     *
     * @return true success 成功
     * @return false failed 失败
     */
    bool unloadAndRemoveFile(String currentDataBaseName);

    inline ~MyDB()
    {
        this->unload();
    }
};

extern MyDB db;
extern MyDB dbUser;
extern MyDB dbApp;

#endif