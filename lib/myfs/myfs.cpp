#include "myfs.h"

void MyFS::myfsInit()
{
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
    {
        delay(1000);
        ESP_LOGD("myfs", "formatted");
    }
}

void MyFS::listFile(String path, fileElementList *list, String prefix)
{
    File root = LittleFS.open(path);
    if (prefix != "")
    {
        prefix = "/" + prefix;
    }

    if (root)
    {
        if (root.isDirectory())
        {
            File file = root.openNextFile();

            while (file)
            {
                FileElement fileInfo;
                fileInfo.name = file.name();
                if (file.isDirectory())
                {
                    fileInfo.isFile = false;
                    fileInfo.size = 0;
                    list->push_back(fileInfo);
                }
                else
                {
                    if (prefix != "")
                    {
                        if (!fileInfo.name.startsWith(prefix))
                        {
                            file = root.openNextFile();
                            continue;
                        }
                    }
                    fileInfo.isFile = true;
                    fileInfo.size = file.size();
                    list->push_back(fileInfo);
                }
                file = root.openNextFile();
            }
        }
    }
}

bool MyFS::writeFile(const char *p, const char *data, bool base64Encode)
{
    if (!base64Encode)
    {
        return writeFile(p, (uint8_t *)data, strlen(data));
    }
    else
    {
        char *d = mycrypto::Base64::base64Encode((uint8_t *)data, strlen(data));
        return writeFile(p, (uint8_t *)d, strlen(d));
    }
}

bool MyFS::writeFile(const char *p, uint8_t *data, uint64_t length)
{
    String path = p;
    if (path[0] != '/')
    {
        path = "/" + path;
    }

    File file = LittleFS.open(path, FILE_WRITE);
    if (!file)
    {
        return false;
    }

    bool value = file.write(data, length);
    file.flush();
    file.close();
    return value;
}

bool MyFS::writeFile(String path, String data, bool base64Encode)
{
    if (path[0] != '/')
    {
        path = "/" + path;
    }

    File file = LittleFS.open(path, FILE_WRITE);
    if (!file)
    {
        return false;
    }

    if (base64Encode)
    {
        data = mycrypto::Base64::base64Encode(data);
    }

    bool value = file.print(data);
    file.close();
    return value;
}

uint8_t *MyFS::readFile(const char *p, uint64_t *outLen)
{
    String path = p;
    if (path[0] != '/')
    {
        path = "/" + path;
    }
    File file = LittleFS.open(path);

    if (!file || file.isDirectory())
    {
        (*outLen) = 0;
        return nullptr;
    }

    uint64_t size = file.size();
    uint8_t *data = new (std::nothrow) uint8_t[size];

    if (!data)
    {
        (*outLen) = 0;
        return nullptr;
    }
    file.readBytes((char *)data, size);
    file.close();

    (*outLen) = size;
    return data;
}

void MyFS::readFile(const char *p, std::function<void(uint8_t *output, uint64_t length)> callback)
{
    String path = p;
    if (path[0] != '/')
    {
        path = "/" + path;
    }
    File file = LittleFS.open(path);

    if (!file || file.isDirectory())
    {
        callback(nullptr, 0);
        return;
    }
    uint64_t size = 0;

    uint8_t *data = MyFS::readFile(p, &size);

    callback(data, size);

    delete data;
}

String MyFS::readFile(String path, bool base64Decode)
{
    if (path[0] != '/')
    {
        path = "/" + path;
    }

    File file = LittleFS.open(path);

    if (!file)
    {
        return String("error");
    }

    if (file.isDirectory())
    {
        return String("not file");
    }

    uint32_t length = file.size();
    uint8_t *data = new (std::nothrow) uint8_t[length + 1];

    if (!data)
    {
        return String("memory full");
    }

    file.readBytes((char *)data, length);
    file.close();
    data[length] = 0;
    String output = String((char *)data);

    if (base64Decode)
    {
        output = mycrypto::Base64::base64Decode(output);
    }

    return output;
}

bool MyFS::appendFile(String path, String data)
{
    if (path[0] != '/')
    {
        path = "/" + path;
    }

    File file = LittleFS.open(path, FILE_APPEND);

    if (!file)
    {
        return false;
    }

    bool value = file.print(data);
    file.close();

    return value;
}

bool MyFS::deleteFile(String path)
{

    if (path[0] != '/')
    {
        path = "/" + path;
    }
    return LittleFS.remove(path);
}

bool MyFS::renameFile(String path0, String path1)
{
    if (path0[0] != '/')
    {
        path0 = "/" + path0;
    }
    if (path1[0] != '/')
    {
        path1 = "/" + path1;
    }

    return LittleFS.rename(path0, path1);
}

bool MyFS::fileExist(String path)
{
    if (path[0] != '/')
    {
        path = "/" + path;
    }

    return LittleFS.exists(path);
}

void MyFS::formatSPIFFS()
{
    LittleFS.format();
}

size_t MyFS::getFreeSpace()
{
    return LittleFS.totalBytes() - LittleFS.usedBytes();
}

size_t MyFS::getUsedSpace()
{
    return LittleFS.usedBytes();
}