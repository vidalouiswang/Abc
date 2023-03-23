#include "mydb.h"

std::vector<Element *> *MyDB::readFile(bool mainFile)
{
    uint64_t outLen = 0;
    uint8_t *buffer = nullptr;
    String fileName = mainFile ? (this->name + ".db") : (this->name + ".bak");

    if (!MyFS::fileExist(fileName))
    {
        return nullptr;
    }

    buffer = MyFS::readFile(fileName.c_str(), &outLen);

    if (outLen && buffer)
    {
        std::vector<Element *> *list = ArrayBuffer::decodeArrayBuffer(buffer, outLen);
        delete buffer;
        return list;
    }
    if (buffer)
    {
        delete buffer;
    }
    return nullptr;
}

void MyDB::removeBuffer(std::vector<Element *> *list)
{
    for (uint32_t i = 0; i < list->size(); ++i)
    {
        delete list->at(i);
    }
    delete list;
}

void MyDB::buildContainer(std::vector<Element *> *list)
{
    if (!list)
        return;

    std::vector<Element *>::iterator it = list->begin();
    std::vector<Element *>::iterator end = list->end();

    for (; it != end;)
    {
        ESP_LOGD(MYDB_DEBUG_HEADER, "list-> %s", (*it)->c_str());
        Unit *u = new Unit((*it));
        ++it;
        if (it == end)
        {
            u->value = new Element();
            this->container->push_back(u);
            break;
        }
        u->value = (*it);
        ++it;
        this->container->push_back(u);
    }
}

bool MyDB::begin()
{
    ESP_LOGD(MYDB_DEBUG_HEADER, "mydb begining, db name: [%s]", this->name.c_str());
    this->container = new std::vector<Unit *>();
    this->loaded = true;

    std::vector<Element *> *list = nullptr;

    // read main file
    list = this->readFile();

    if (!list)
    {
        // read backup file
        list = this->readFile(false);
    }
    if (!list)
    {
        return false;
    }

    if (list->size())
    {
        if (list->size() % 2)
        {
            this->removeBuffer(list);
            list = nullptr;
        }
    }
    else
    {
        this->removeBuffer(list);
        list = nullptr;
    }

    // no error
    this->buildContainer(list);
    list->clear();
    delete list;
    return true;
}

bool MyDB::makeBackup()
{
    String mainFileName = this->name + ".db";
    String backupName = this->name + ".bak";
    if (MyFS::fileExist(mainFileName))
    {
        uint64_t outLen = 0;
        uint8_t *buffer = MyFS::readFile(mainFileName.c_str(), &outLen);
        if (outLen)
        {
            if (buffer)
            {
                MyFS::writeFile(backupName.c_str(), buffer, outLen);
                delete buffer;
                return true;
            }
        }
    }
    return false;
}

void MyDB::dump(uint8_t **buffer, uint64_t *outLen)
{
    (*outLen) = 0;

    if (!this->loaded)
        return;

    ESP_LOGD(MYDB_DEBUG_HEADER, "database container length: %lu", this->container->size());

    if (!this->container->size())
        return;

    std::vector<Unit *>::iterator it = this->container->begin();
    std::vector<Unit *>::iterator end = this->container->end();

    std::vector<Element *> list;

    for (; it != end; ++it)
    {
        if ((*it)->key->available() && (*it)->value->available())
        {
            list.push_back((*it)->key);
            list.push_back((*it)->value);
        }
        // don't delete those removed units here
    }

    ESP_LOGD(MYDB_DEBUG_HEADER, "dump container length: %u", list.size());

    *buffer = ArrayBuffer::createArrayBuffer(&list, outLen);
}

bool MyDB::flush()
{
    if (!this->loaded)
        return false;

    if (!this->makeBackup())
    {
        ESP_LOGD(MYDB_DEBUG_HEADER, "unable to create backup");
    }
    uint8_t *buffer = nullptr;
    uint64_t outLen = 0;

    this->dump(&buffer, &outLen);
    ESP_LOGD(MYDB_DEBUG_HEADER, "dump buffer created, length: %llu", outLen);

    if (outLen)
    {
        if (buffer)
        {
            ESP_LOGD(MYDB_DEBUG_HEADER, "buffer valid");
            bool flushOK = MyFS::writeFile((this->name + ".db").c_str(), buffer, outLen);
            if (!flushOK)
            {
                ESP_LOGD(MYDB_DEBUG_HEADER, "unable to flush");
            }
            delete buffer;
            return flushOK;
        }
    }
    return false;
}

int64_t MyDB::findIndex(Element *key)
{
    if (!this->container)
        return -2;

    if (!this->container->size())
        return -2;

    uint32_t length = this->container->size();

    // ESP_LOGD(MYDB_DEBUG_HEADER, "length of container: %lu", length);

    for (uint32_t i = 0; i < length; ++i)
    {
        // ESP_LOGD(MYDB_DEBUG_HEADER, "left address: %x, right address: %x", eKey, key);

        if (this->container->at(i)->key->equalsTo(key))
        {
            return i;
        }
    }
    return -2;
}

Element *MyDB::operator()(const char *data)
{
    if (!this->loaded)
        return nullptr;

    if (!strlen(data))
        return nullptr;

    Element *eKey = new Element(data);

    Element *value = this->operator()(eKey);

    if (value->available())
        delete eKey;

    return value;
}

Element *MyDB::operator()(Element *key)
{
    if (!this->loaded)
        return nullptr;

    if (!key)
        return nullptr;
    if (!key->available())
        return nullptr;

    int64_t index = this->findIndex(key);
    if (index < 0)
    {
        ESP_LOGD(MYDB_DEBUG_HEADER, "unit does not exists [%s]", key->c_str());
        Unit *unit = new Unit(key, new Element());
        ESP_LOGD(MYDB_DEBUG_HEADER, "new unit created");

        if (!this->container)
        {
            this->container = new std::vector<Unit *>();
        }

        this->container->push_back(unit);
        return unit->value;
    }
    else
    {
        ESP_LOGD(MYDB_DEBUG_HEADER, "unit exists, index: %lu", index);
        return this->container->at(index)->value;
    }
}

void MyDB::unload()
{
    if (this->loaded)
    {
        if (this->container)
        {
            if (this->container->size())
            {
                for (uint32_t i = 0; i < this->container->size(); ++i)
                {
                    delete this->container->at(i);
                }
            }
            delete this->container;
        }
    }
    this->loaded = false;
}

bool MyDB::unloadAndRemoveFile(String currentDataBaseName)
{
    if (currentDataBaseName != this->name)
        return false;

    this->unload();

    MyFS::deleteFile(this->name + ".db");
    MyFS::deleteFile(this->name + ".bak");

    return true;
}

MyDB db("mydb");
MyDB dbUser("dbUser");
MyDB dbApp("dbApp");