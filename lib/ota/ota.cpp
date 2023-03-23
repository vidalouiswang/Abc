#include "ota.h"
#include <mydb.h>

WebsocketOTA::WebsocketOTA(std::vector<Element *> *startOTARequest, OTACallback startCallback, OTACallback abortCallback)
    : startCallback(startCallback), abortCallback(abortCallback)
{
    // server send websocket OTA request
    /*
        0 0xab, //websocket ota
        1 json.admin, //web client id
        2 segmentSize, //block size
        3 json.data.length, //total size
        4 arr[4], //time
        5 arr[5] //hash
    */

    // length of firmware
    this->firmwareLength = startOTARequest->at(OFFSET_START_OTA_FIRMWARE_LENGTH)->getUint64();

    // single block size
    this->blockLength = startOTARequest->at(OFFSET_START_OTA_BLOCK_SIZE)->getUint64();

    // request index
    this->index = 0;
}

void WebsocketOTA::start(Element *universalID, String domain, uint16_t port, String path)
{
    this->client = new WebSocketClient();

    // ota request container
    // ota 请求容器
    this->request = new std::vector<Element *>();

    // prefill data
    // 预填充数据
    this->request->push_back(new Element(CMD_OTA_BLOCK));
    this->request->push_back(new Element(universalID->getUint8Array(), 32));
    this->request->push_back(new Element(this->index));

    // set callback for ota update websocket client
    // 为ota升级websocket客户端设置回调函数
    this->client->setCallBack(
        [this](WebSocketEvents event, uint8_t *data, uint64_t length)
        {
            if (event == WS_CONNECTED)
            {
                // fetch data block if is in websocket ota mode
                // 当socket断开又重新连接时继续请求数据
                this->fetchNext();
            }
            else if (event == TYPE_BIN && data && length)
            {
                // using non-copy mode decode buffer
                // 使用非拷贝模式解码原始数据
                std::vector<Element *> *output = ArrayBuffer::decodeArrayBuffer(data, length, true);

                // run data checks and write data to partition down bellow
                // no need more comments
                // 下面执行了各种数据检测然后写入ota分区，比较简单就不写注释了

                if (!output->size())
                {
                    this->clearBuffer(output);
                    return;
                }

                if (output->at(OFFSET_COMMAND)->getType() != ETYPE_UINT8 ||
                    output->at(OFFSET_COMMAND)->getUint8() != CMD_OTA_BLOCK)
                {
                    this->clearBuffer(output);
                    return;
                }

                if (output->size() != OTA_DATA_BLOCK_VECTOR_LENGTH)
                {
                    ESP_LOGD(OTA_DEBUG_HEADER, "invalid length of arguments, someone may send hack data");
                    this->clearBuffer(output);
                    this->fetchNext();
                    return;
                }

                if (
                    output->at(OFFSET_OTA_DATA)->getType() != ETYPE_BUFFER)
                {
                    ESP_LOGD(OTA_DEBUG_HEADER, "invalid type of data or hash");
                    this->clearBuffer(output);
                    this->fetchNext();
                    return;
                }

                if (output->at(OFFSET_OTA_BLOCK_INDEX)->getType(true) != ETYPE_NUMBER)
                {
                    ESP_LOGD(OTA_DEBUG_HEADER, "invalid type of index");
                    this->clearBuffer(output);
                    this->fetchNext();
                    return;
                }

                if (output->at(OFFSET_OTA_BLOCK_INDEX)->getUint32() < this->index)
                {
                    ESP_LOGD(OTA_DEBUG_HEADER, "repetitive block");
                    this->clearBuffer(output);
                    this->fetchNext();
                    return;
                }

                // server send OTA data block
                /*
                    index:
                    1 == sha256, uint8 array, 32 bytes
                    2 == data, uint8 array
                */
                ESP_LOGD(OTA_DEBUG_HEADER, "block arrived, length: %d\n", output->at(2)->getU8aLen());

                if (!output->at(OFFSET_OTA_DATA)->getU8aLen())
                {
                    // empty block means all data has been written
                    // OTA update finished
                    if (ESP_OK == esp_ota_end(this->handle) &&
                        ESP_OK == esp_ota_set_boot_partition(this->partition))
                    {
                        ESP_LOGD(OTA_DEBUG_HEADER, "Websocket OTA update successfully");

                        // set OTA update mark
                        // 设置OTA更新标志
                        *(db("pendingOTA")) = 0x01;

                        // reset boot count
                        // 重置启动次数计数
                        *(db("pendingOTABootCount")) = 0x00;

                        // clear former firmware invalid mark(if exists)
                        // 如果以前曾发生过回滚，清除回滚标志
                        *(db("pendingOTAInvalid")) = 0x00;

                        db.flush();
                        delay(1000);
                    }
                    else
                    {
                        ESP_LOGD(OTA_DEBUG_HEADER, "Websocket OTA update failed");
                        esp_ota_abort(this->handle);
                    }
                    ESP.restart();
                }

                // calc hash of data block
                String localHash = mycrypto::SHA::sha256(
                    output->at(OFFSET_OTA_DATA)->getUint8Array(),
                    output->at(OFFSET_OTA_DATA)->getU8aLen());

                if (localHash == output->at(OFFSET_OTA_HASH)->getString())
                {
                    // write data to ota partition
                    if (ESP_OK ==
                        esp_ota_write_with_offset(
                            this->handle,                                               // ota partition handle
                            (const void *)output->at(OFFSET_OTA_DATA)->getUint8Array(), // data buffer
                            output->at(OFFSET_OTA_DATA)->getU8aLen(),                   // length of buffer
                            this->writeOffset                                           // offset
                            ))
                    {
                        ++this->index;
                        this->writeOffset += output->at(OFFSET_OTA_DATA)->getU8aLen();
                        this->clearBuffer(output);
                        this->fetchNext();
                    }
                    else
                    {
                        this->clearBuffer(output);
                        // unknown reason can't write data into flash
                        // abort ota process
                        esp_ota_abort(this->handle);

                        // call abort callback
                        this->abortCallback(1);
                    }
                }
                else
                {
                    // hash failed, request last block again
                    // todo: add retry count detect
                    this->clearBuffer(output);
                    this->fetchNext();
                }
            }
        });

    // get next ota partition
    this->partition = esp_ota_get_next_update_partition(NULL);

    ESP_LOGD(OTA_DEBUG_HEADER, "OTA partition start(hex): %x\n", this->partition->address);

    if (ESP_OK == esp_ota_begin(this->partition, this->firmwareLength, &(this->handle)))
    {
        ESP_LOGD(OTA_DEBUG_HEADER, "OTA update start successfully");

        this->otaInitialized = true;
    }
    else
    {
        this->aborted = true;
        this->startCallback(1);
    }

    if (!this->aborted)
    {
        this->client->connect(domain, port, path);
    }
}

void WebsocketOTA::clearBuffer(std::vector<Element *> *output)
{
    for (std::vector<Element *>::iterator it = output->begin();
         it != output->end();
         ++it)
    {
        delete (*it);
    }
    delete output;
}

void WebsocketOTA::fetchNext()
{
    // fill ota data block index
    this->request->at(2)->setNumber(this->index);

    // create buffer
    uint8_t *buffer = ArrayBuffer::createArrayBuffer(this->request, &(this->bufferOutLen));

    ESP_LOGD(OTA_DEBUG_HEADER, "fetching block: %d, heap: %lu\n", this->index, ESP.getFreeHeap());

    // send buffer
    this->client->send(buffer, this->bufferOutLen);

    delete buffer;
}

WebsocketOTA::~WebsocketOTA()
{
    if (this->client)
    {
        delete this->client;
    }
}