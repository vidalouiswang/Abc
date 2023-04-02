#include "globalmanager.h"
#include <languages.h>
#include <myfs.h>
#include <mynet.h>
#include <esp32time.h>

void otaTask(void *t)
{
    for (;;)
    {
        global->ota->client->loop();
    }
    vTaskDelete(NULL);
}

void APScheduler(void *t)
{
    for (;;)
    {
        if (global->isAPStarted)
        {
            if (millis() - global->getRemoteWebsocketConnectedTime() > AUTOMATIC_CLOSE_AP_IF_REMOTE_WEBSOCKET_CONNECTED_TIMEOUT)
            {
                ESP_LOGD(SYSTEM_DEBUG_HEADER, "AP closed");
                global->closeAP();
            }
        }
        else
        {
            if (!global->doNotEnableAP &&
                global->serverOfflineTimes() > AUTOMATIC_START_AP_IF_REMOTE_WEBSOCKET_DISCONNECTED_TIMES)
            {
                global->startAP();
            }

            if (WiFi.getMode() > 1)
            {
                global->globalTask |= GT_CLOSE_AP;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}

void setup()
{
    global->beginAll();
}

void loop()
{
    global->loop();
}

void GlobalManager::beginAll(WebSocketCallback apCB,
                             WebSocketCallback wifiCB)
{

#ifdef APP_REQUIRE_PREPARE_SETUP
    app->preSetup();
#endif

#ifdef APP_HAS_EXTRA_LOCAL_WEBSOCKET_CALLBACK
    // extra local(AP) websocket callback
    // if built-in command missed, it will be called
    // 额外的 本地(AP) websocket 回调函数
    // 如果内置命令没有命中，这个回调函数会被执行
    global->setExtraLocalWebsocketCallback(extraLocalWebsocketCallback);
#endif
#ifdef APP_HAS_EXTRA_REMOTE_WEBSOCKET_CALLBACK
    // extra remote(WiFi) websocket callback
    // if built-in command missed, it will be called
    // 额外的 远程(WiFi) websocket 回调函数
    // 如果内建命令没有命中，这个回调函数会被执行
    global->setExtraRemoteWebsocketCallback(extraRemoteWebsocketCallback);
#endif

    // hardware related settings must be set at first
    // set cpu frequency
    setCpuFrequencyMhz(DEFAULT_CPU_FREQ);

    // enable sha hardware acceleration of esp32
    mycrypto::SHA::initialize();

    // enable aes hardware acceleration of esp32
    mycrypto::AES::initialize();

    // initialize littlefs
    this->beginFS();

    // load main database raw data(if exists) and build database in ram
    this->beginMainDB();

    // this will make sure new firmware valid, or it will call rollback
    this->makeSureNewFirmwareValid();

    // load other database
    this->beginOtherDB();

    // generate device ID
    this->beginUniversalID();

    ESP_LOGD(SYSTEM_DEBUG_HEADER, "Run App Setup");
    app->setup();

    // fill callbacks
    if (apCB)
        this->extraAPWebsocketCallback = apCB;

    if (wifiCB)
        this->extraWifiWebsocketCallback = wifiCB;

    // load some basic information and start ap
    this->initializeBasicInformation();

    ESP_LOGD(SYSTEM_DEBUG_HEADER, "Current Structure Version: %d", SYSTEM_VERSION);

    this->buildProvidersBuffer();

#ifdef APP_VERSION
    ESP_LOGD(SYSTEM_DEBUG_HEADER, "Current App Version: %d", APP_VERSION);
#endif

#ifdef SINGLE_TASK_RUN_MAINLOOP
    // create main loop task
    xTaskCreatePinnedToCore(mainLoop,                // function
                            "mainLoop",              // name
                            MAIN_LOOP_STACK_SIZE,    // stack size
                            NULL,                    // input argument
                            MAIN_LOOP_TASK_PRIORITY, // priority
                            NULL,                    // handle
                            MAIN_LOOP_TASK_CORE      // core 0 / 1
    );
#endif

    this->setSerialRecvCb();

    Serial.println("OK!");
}

void GlobalManager::syncTime(uint64_t t)
{
    globalTime->setTime(t);
    if (!this->systemPowerOnTime)
        this->systemPowerOnTime = t;
}

void GlobalManager::buildProvidersBuffer(bool buildAll)
{
    ESP_LOGD(SYSTEM_DEBUG_HEADER, "Building provider bufffer");
    std::vector<Element *> providerBufferContainer;

    uint8_t *providerBuffer = nullptr;
    uint32_t providerOutLen = 0;

    for (
        std::vector<Provider *>::iterator it = this->providers->begin();
        it != this->providers->end();
        ++it)
    {
        if (!buildAll && (*it)->isBuiltIn)
        {
            // if system require build only normal
            // providers, then skip built-in providers
            continue;
        }

        providerBuffer = (*it)->getBuffer(&providerOutLen);
        if (providerBuffer && providerBuffer)
        {
            providerBufferContainer.push_back(new Element(providerBuffer, providerOutLen));
            delete providerBuffer;
            providerBuffer = nullptr;
        }
    }

    ESP_LOGD(SYSTEM_DEBUG_HEADER, "Providers buffer container had been built, length: %d", providerBufferContainer.size());

    if (providerBufferContainer.size())
    {
        if (this->bufferProviders)
        {
            delete this->bufferProviders;
        }

        this->isProviderBufferShrank = buildAll ? false : true;

        this->bufferProviders = ArrayBuffer::createArrayBuffer(&providerBufferContainer, &(this->bufferProvidersLength));

        ESP_LOGD(SYSTEM_DEBUG_HEADER, "Providers buffer had been built");

        for (uint32_t i = 0; i < providerBufferContainer.size(); ++i)
        {
            delete providerBufferContainer.at(i);
        }

        ESP_LOGD(SYSTEM_DEBUG_HEADER, "Providers buffer container cache had been removed");
    }
    else
    {
        ESP_LOGD(SYSTEM_DEBUG_HEADER, "None provier provide nickname");
    }
}

void GlobalManager::buildUsersBuffer()
{

    if (dbUser.count() > 0)
    {
        ESP_LOGD(SYSTEM_DEBUG_HEADER, "\n\nthere are %lu other users\n\n", dbUser.count());
        dbUser.dump(&(this->usersBuffer), &(this->lengthOfUsersBuffer));
    }
}

void GlobalManager::startAP()
{
    if (this->doNotEnableAP || this->isAPStarted)
    {
        return;
    }
    delay(1);

    // read custom prefix for local access point
    String prefix = db("apPrefix")->getString();

    // use nickname as ap ssid name if set, or use prefix 8 bytes as ssid
    String ssid = (prefix.length() ? prefix : (String(AP_PREFIX) + "_")) +
                  (this->nickname.getType() == ETYPE_STRING ? this->nickname.getString().c_str()
                                                            : this->nickname.getHex().substring(0, 8));

    ESP_LOGD(SYSTEM_DEBUG_HEADER, "starting AP");
    // if current esp32 has been setup once
    // it will use user password(sha256 hashed) as ap password
    if (this->password.available())
    {
        String apPassword = this->password.getHex();
        ESP_LOGD(SYSTEM_DEBUG_HEADER, "ap password: %s", apPassword.c_str());
        myNet.startAP(ssid.c_str(), this->apIP, apPassword.c_str());
    }
    else
    {
// public ap
#ifdef PRESET_AP_PASSWORD
        ESP_LOGD(SYSTEM_DEBUG_HEADER, "ssid:%s, password:%s", ssid.c_str(), PRESET_AP_PASSWORD);
        myNet.startAP(ssid.c_str(), this->apIP, PRESET_AP_PASSWORD);
#else
        myNet.startAP(ssid.c_str(), this->apIP);
#endif
    }
    ESP_LOGD(SYSTEM_DEBUG_HEADER, "AP started");

    if (!this->apServer)
    {
        this->apServer = new myWebSocket::CombinedServer();
    }

    // setup basic websocket callback for local server
    apServer->setCallback(
        [this](myWebSocket::WebSocketClient *client,
               myWebSocket::WebSocketEvents event,
               uint8_t *data,
               uint64_t length)
        {
            // call universal websocket callback
            this->internalUniversalWebsocketCallback(client, event, data, length);
        });

    // setup http callback
    apServer
        ->on("/",
             [this](myWebSocket::ExtendedWiFiClient *client, myWebSocket::HttpMethod method, uint8_t *data, uint64_t length)
             {
                 // send default page
                 client->send(this->serverIndex);
                 client->close();
             });

    // start combined server
    apServer->begin(80);

    // dns
    // dnsServer = new DNSServer();
    // dnsServer->start(53, "*", apIP);
    // print("dns server online");

    this->isAPStarted = true;

    ESP_LOGD(SYSTEM_DEBUG_HEADER, "Local server online");
}

void GlobalManager::makeSureNewFirmwareValid()
{
    Element pendingOTA = db("pendingOTA");

    if (pendingOTA.available())
    {
        Element pendingOTABootCount = db("pendingOTABootCount");
        if (pendingOTABootCount.available())
        {
            if (pendingOTA.getNumber() == 0x01)
            {
                this->isNewFirmwareBoot = true;
                if (pendingOTABootCount.getNumber() >= ERROR_BOOT_COUNT_VALVE)
                {
                    ESP_LOGD(SYSTEM_DEBUG_HEADER, "New firmware has error, rollback");
                    (*(db("pendingOTA"))) = 0;
                    (*(db("pendingOTAInvalid"))) = globalTime->getTime();
                    (*(db("pendingOTABootCount"))) = 0;
                    db.flush();
                    delay(100);
                    esp_ota_mark_app_invalid_rollback_and_reboot();
                }
                else
                {
                    (*(db("pendingOTABootCount"))) = pendingOTABootCount.getNumber() + 1;
                    db.flush();
                }
            }
        }
    }
}

void GlobalManager::markNewFirmwareIsValid()
{
    this->isNewFirmwareBoot = false;
    (*(db("pendingOTA"))) = 0;
    (*(db("pendingOTABootCount"))) = 0;
    (*(db("lastOTATime"))) = globalTime->getTime();
    db.flush();
    ESP_LOGD(SYSTEM_DEBUG_HEADER, "New firmware is valid");
}

void GlobalManager::executeCommand(
    std::vector<Element *> *output,
    std::vector<Element *> *response)
{
    /*
        1 == esp32 id
        2 == web client id
        3 == time
        4 == hash
        5 == uint8 array{
            0, string, command name,
            from 1, arguments...
        }
    */
    bool isAdmin = false;

    if ((output->size() == 7 || output->size() == 6) &&
        output->at(4)->getType() == ETYPE_BUFFER &&
        output->at(4)->getRawBufferLength() == SHA_LENGTH &&
        this->authorize(output->at(4), output->at(3), &isAdmin))
    {
        ESP_LOGD(SYSTEM_DEBUG_HEADER, "authorized");

        Element *result = nullptr;
        ProviderArguments arguments = nullptr;

        // fill response
        response->push_back(new Element(CMD_LOG));
        response->push_back(new Element(this->getUniversalID().getRawBuffer(), 32));
        response->push_back(new Element(output->at(2)->getString()));
        response->push_back(new Element("unavailable"));

        int providerIndex = -2;

        // find function provider and get result

        uint64_t idFromRemote = output->at(5)->getNumber();

        for (uint32_t i = 0; i < this->providers->size(); ++i)
        {
            uint64_t targetID = output->at(5)->getType() == ETYPE_UINT64
                                    ? this->providers->at(i)->customID
                                    : this->providers->at(i)->id;
            if (targetID == idFromRemote)
            {
                providerIndex = i;
                break;
            }
        }

        if (providerIndex < 0)
        {
            return;
        }
        uint8_t *key = this->password.getUint8Array();
        uint8_t *iv = this->password.getUint8Array();
        uint32_t aesOutLen = 0;

        if (output->size() == 7)
        {
            if (output->at(6)->getType() != ETYPE_BUFFER)
            {
                ESP_LOGD(SYSTEM_DEBUG_HEADER, "data index 6 has invalid type");
                return;
            }
            if (this->providers->at(providerIndex)->encrypt)
            {
                uint8_t *buffer = mycrypto::AES::aes256CBCDecrypt(key,
                                                                  iv,
                                                                  output->at(6)->getUint8Array(),
                                                                  output->at(6)->getRawBufferLength(),
                                                                  &aesOutLen);

                if (aesOutLen && buffer)
                {
                    arguments = ArrayBuffer::decodeArrayBuffer(buffer, aesOutLen);
                    delete buffer;
                }
            }
            else
            {
                arguments = ArrayBuffer::decodeArrayBuffer(output->at(6)->getUint8Array(),
                                                           output->at(6)->getRawBufferLength());
            }
        }
        else
        {
            arguments = new std::vector<Element *>();
        }

        // run callback
        result = this->providers->at(providerIndex)->cb(arguments);

        // clean
        for (uint32_t i = 0; i < arguments->size(); ++i)
        {
            delete arguments->at(i);
        }
        delete arguments;

        // fill result
        if (result)
        {
            if (result->available())
            {
                std::vector<Element *> container = {result};
                uint32_t outLen = 0;
                uint8_t *buffer = ArrayBuffer::createArrayBuffer(&container, &outLen);
                delete result;

                if (outLen && buffer)
                {
                    if (this->providers->at(providerIndex)->encrypt)
                    {
                        aesOutLen = 0;
                        uint8_t *encryptedBuffer = mycrypto::AES::aes256CBCEncrypt(key, iv, buffer, outLen, &aesOutLen);

                        if (aesOutLen && encryptedBuffer)
                        {
                            response->at(3)->copyFrom(encryptedBuffer, aesOutLen);
                            response->push_back(new Element(0x10));
                            delete encryptedBuffer;
                        }
                    }
                    else
                    {
                        response->at(3)->copyFrom(buffer, outLen);
                    }
                    delete buffer;
                }
                else
                {
                    (*(response->at(3))) = "error when execute function";
                }
            }
        }
    }
}

void GlobalManager::sendHello(bool hello, myWebSocket::WebSocketClient *client)
{
    uint8_t buffer[2] = {0x01, ((hello ? CMD_HELLO : CMD_WORLD))};

    myWebSocket::WebSocketClient *sender = client ? (client)
                                                  : ((this->websocketClient ? (this->websocketClient) : (nullptr)));

    if (sender)
        sender->send(buffer, 2);
}

void GlobalManager::commandFromEspNow(uint8_t *data, uint32_t length)
{
}

void GlobalManager::routeDataToWebsocketHandler(uint8_t *data,
                                                uint32_t length,
                                                myWebSocket::WebSocketClient *client,
                                                myWebSocket::WebSocketEvents event)
{
    if (!data || !length)
    {
        return;
    }

    // decode buffer and call message handler
    std::vector<Element *> *output = ArrayBuffer::decodeArrayBuffer(data, length);

    //  call handler
    if (!client)
    {
        this->internalRemoteMsgHandler(event, data, length, output);
    }
    else
    {
        this->internalLocalMsgHandler(client, event, data, length, output);
    }

    for (std::vector<Element *>::iterator it = output->begin();
         it != output->end();
         ++it)
    {
        delete (*it);
    }
    delete output;
}

void GlobalManager::connectWifi()
{
    ESP_LOGD(SYSTEM_DEBUG_HEADER, "Connecting to WiFi");
    String wifiSSID = db("wifiSSID")->getString(),
           wifiPwd = db("wifiPwd")->getString();

    if (!wifiSSID.length() || !wifiPwd.length() || wifiPwd.length() < 8)
    {
#ifdef PRESET_WIFI_SSID
#ifdef PRESET_WIFI_PASSWORD
        wifiSSID = String(PRESET_WIFI_SSID);
        wifiPwd = String(PRESET_WIFI_PASSWORD);
#else
        return;
#endif
#else
        return;
#endif
    }

    if (this->doNotEnableAP)
    {
        if (WiFi.getMode() != WIFI_MODE_STA)
        {
            WiFi.mode(WIFI_MODE_STA);
        }
    }
    else
    {
        if (WiFi.getMode() != WIFI_MODE_APSTA)
        {
            WiFi.mode(WIFI_MODE_APSTA);
        }
    }

    // connect wifi
    WiFi.begin(wifiSSID.c_str(), wifiPwd.c_str());

    WiFi.setAutoReconnect(false);

    if (!(this->fnWiFiSTAGotIP))
    {
        this->fnWiFiSTAGotIP = [](arduino_event_t *event)
        {
            ESP_LOGD(SYSTEM_DEBUG_HEADER, "WiFi connected");
            global->setWiFiStatus(true);
            if (!global->remoteWebsocketConnectedTimestamp)
            {
                global->connectWebocket();
            }
            WiFi.removeEvent(global->fnWiFiSTAGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
            if (!global->fnWiFiSTADissconnected)
            {
                global->fnWiFiSTADissconnected = [](arduino_event_t *event)
                {
                    ESP_LOGD(SYSTEM_DEBUG_HEADER, "WiFi disconnected, will retry to connect every %u milliseconds",
                             AUTO_CONNECT_WIFI_TIMEOUT);
                    global->setWiFiStatus(false);
                    WiFi.removeEvent(global->fnWiFiSTADissconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
                };
            }
            WiFi.onEvent(global->fnWiFiSTADissconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        };
    }

    WiFi.onEvent(this->fnWiFiSTAGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
};

void GlobalManager::refreshData()
{
    // refresh data
    this->isWifiEnabled = this->isWifiEnabled ? 0xffu : 0;
    this->isWifiConnected = this->isWifiConnected ? 0xffu : 0;
    this->isWiFiInfoOK = this->isWiFiInfoOK ? 0xffu : 0;
    if (!this->ota)
    {
        this->ota = nullptr;
    }
}

void GlobalManager::internalLocalMsgHandler(
    myWebSocket::WebSocketClient *client,
    myWebSocket::WebSocketEvents event,
    uint8_t *data,
    uint64_t length,
    std::vector<Element *> *output)
{
    // check size
    if (output->size() <= 0)
        return;

    // check command type
    if (output->at(0)->getType() != ETYPE_UINT8)
        return;

    // get length of data vector
    uint8_t lengthOfOutput = output->size();

    // get command
    uint8_t command = output->at(0)->getUint8();

    // response container
    std::vector<Element *> *response = new std::vector<Element *>();

    switch (command)
    {
    case CMD_AP_SET_BASIC_INFORMATION:
        // set wifi info from
        // 1 == nickname, string
        // 2 == ssid, string
        // 3 == wifi password, string
        // 4 == user name, uint8 array
        // 5 == user password, uint8 array
        // 6 == websocket server url, string
        // 7 == websocket server port, uint16
        // 8 == websocket path, string
        {
            if (lengthOfOutput != 10)
            {
                ESP_LOGD(SYSTEM_DEBUG_HEADER, "invalid length of arguments");
                delete response;
                return;
            }

            // update information to database

            // nickname
            if (output->at(1)->getType() == ETYPE_STRING)
                (*(db)("nickname")) = output->at(1)->getString();
            // db->update("nickname", output->at(1)->getString());

            // wifi information
            if (output->at(2)->getType() == ETYPE_STRING && output->at(3)->getType() == ETYPE_STRING)
            {
                (*(db)("wifiSSID")) = output->at(2)->getString();
                (*(db)("wifiPwd")) = output->at(3)->getString();
            }

            // for remote management (admin)
            if (output->at(4)->getType() == ETYPE_BUFFER && output->at(5)->getType() == ETYPE_BUFFER)
            {
                (*(db)("userName"))(output->at(4)->getUint8Array(), output->at(4)->getRawBufferLength());
                (*(db)("password"))(output->at(5)->getUint8Array(), output->at(5)->getRawBufferLength());
            }

            // websocket information(remote)
            if (output->at(6)->getType() == ETYPE_STRING)
                (*(db)("websocketDomain")) = output->at(6)->getString();

            if (output->at(7)->getType(true) == ETYPE_NUMBER)
                (*(db)("websocketPort")) = (uint16_t)(output->at(7)->getNumber());

            if (output->at(8)->getType() == ETYPE_STRING)
                (*(db)("websocketPath")) = output->at(8)->getString();

            if (output->at(9)->getType() == ETYPE_STRING)
                (*db("token")) = output->at(9)->getString();

            // write to flash
            db.flush();

            // give response to client
            response->push_back(new Element(CMD_AP_SET_BASIC_INFORMATION));

            break;
        }
    case CMD_AP_DELAY_REBOOT:
        // reboot
        {
            // response
            response->push_back(new Element(CMD_AP_DELAY_REBOOT));

            // db->update("rebootReason", "ap manual reboot");
            (*(db)("rebootReason")) = "ap manual reboot";
            db.flush();
            // prepare reboot
            setTimeout(
                []()
                {
                    ESP.restart();
                },
                3000);

            break;
        }
    case CMD_AP_REBOOT_IMMEDIATELY:
        // reboot immediately
        {
            ESP.restart();
            break;
        }
    case CMD_AP_ROLLBACK:
        // rollback
        {
            response->push_back(new Element(CMD_AP_ROLLBACK));

            // db->update("rebootReason", "ap rollback");
            (*(db)("rebootReason")) = "ap rollback";
            db.flush();
            // prepare for rollback firmware
            setTimeout(
                []()
                {
                    esp_ota_mark_app_invalid_rollback_and_reboot();
                },
                3000);
            break;
        }
    case CMD_AP_DEEPSLEEP:
        // deepsleep
        {
            response->push_back(new Element(CMD_AP_DEEPSLEEP));

            // manual deepsleep
            setTimeout(
                []()
                {
                    ESP.deepSleep(600 * 1000 * 1000);
                },
                3000);
            break;
        }
    /*case CMD_AP_CONNECT_WIFI:
        // connect to wifi
        // to check selected wifi if available
        {
            // check size
            if (lengthOfOutput != 3)
            {
                ESP_LOGD(SYSTEM_DEBUG_HEADER, "invalid length of arguments");
                return;
            }

            // check type
            if (output->at(1)->getType() != STRING || output->at(2)->getType() != STRING)
            {
                ESP_LOGD(SYSTEM_DEBUG_HEADER, "invalid type of arguments");
                return;
            }

            // store wifi info
            (*(db)("wifiSSID")) = output->at(1)->getString();
            (*(db)("wifiPwd")) = output->at(2)->getString();
            // db->update("wifiSSID", output->at(1)->getString().c_str());
            // db->update("wifiPwd", output->at(2)->getString().c_str());
            db.flush();

            // fill reponse
            response->push_back(new Element(CMD_AP_CONNECT_WIFI));

            this->connectWifi();

            if ( != WL_CONNECTED)
            {
                response->push_back(new Element(CMD_AP_WIFI_UNAVAILABLE));
            }
            else
            {
                response->push_back(new Element(CMD_AP_WIFI_CONNECTED));
            }

            break;
        }*/
    default:
        this->extraAPWebsocketCallback(client, event, data, length, output);
        break;
    }

    if (response->size())
    {
        // create buffer and send it if response container isn't empty
        ArrayBuffer::createArrayBuffer(
            response,
            [client, response](uint8_t *buffer, uint64_t length, bool *deleted)
            {
                // send buffer
                client->send(buffer, length);

                // delete objs
                for (std::vector<Element *>::iterator it = response->begin();
                     it != response->end();
                     ++it)
                {
                    delete (*it);
                }
            });
    }

    // delete container
    delete response;
}

void GlobalManager::internalRemoteMsgHandler(
    myWebSocket::WebSocketEvents event,
    uint8_t *data,
    uint64_t length,
    std::vector<Element *> *output)
{
    // check size
    if (output->size() <= 0)
        return;

    // get size
    uint8_t lengthOfOutput = output->size();

    // get command
    uint64_t command = output->at(0)->getUint64();

    // extented command
    uint64_t extented = 0;

    // byte offset 0
    // header bytes
    uint8_t header = 0;

    bool msgShouldConfirm = false;

    // byte offset 1, 4 bytes
    // message id
    uint64_t msgID = 0;

    ElementType eType = output->at(0)->getType();

    if (eType == ETYPE_UINT64)
    {
        extented = command & 0xffffffffffffff00ULL;

        header = (uint8_t)(extented >> 56);

        if (header & 0b01000000)
        {
            msgShouldConfirm = true;
            msgID = extented & 0x00ffffffff000000ULL;
        }

        // basic command
        command &= 0xffU;
    }
    else
    {
        if (eType != ETYPE_UINT8)
        {
            return;
        }
    }

    // create response container
    std::vector<Element *> *response = new std::vector<Element *>();

    switch (command)
    {
    case CMD_HELLO:
#ifdef AUTO_CLEAN_BUILT_IN_PROVIDER_BUFFER
        if (!this->isProviderBufferShrank)
        {
            if (globalTime->getTime() - this->lastTimeAdminOnline > TIMEOUT_CLEAN_BUILT_IN_PROVIDER_BUFFER)
            {
                ESP_LOGD(SYSTEM_DEBUG_HEADER, "shrink provider buffer");
                this->buildProvidersBuffer(false);
            }
        }

#endif
        this->sendHello(false);

        this->refreshData();
        break;
    case CMD_WORLD:
        this->serverOnline(1);
        break;
    case CMD_REGISTER_OR_ROLE_AUTHORIZE:
        // server will send current timestamp after esp32 registered
        // server response current time
        // or, server will send 3 uint8 if client authorize is enabled on server
        {
            if (output->size() > 1)
            {
                if (output->size() == 2 && output->at(1)->getType() == ETYPE_UINT64)
                {
                    // sync time
                    this->syncTime(output->at(1)->getUint64());

                    // record power on time
                    if (!this->systemPowerOnTime)
                    {
                        this->systemPowerOnTime = output->at(1)->getUint64();
                    }
                }
                else if (output->size() == 3)
                {
                    if (output->at(1)->getType() == ETYPE_UINT8 &&
                        output->at(2)->getType() == ETYPE_UINT8)
                    {
                        String token = db("token")->getString();
                        if (token.length())
                        {
                            response->push_back(new Element(CMD_REGISTER_OR_ROLE_AUTHORIZE));
                            uint64_t t = globalTime->getTime();
                            char time[24] = {0};
                            sprintf(time, "%llu", t);
                            String hash = token + time;
                            hash = mycrypto::SHA::sha256(hash);
                            Element *eHash = new Element(hash);
                            if (eHash->convertHexStringIntoUint8Array())
                            {
                                response->push_back(new Element(t));
                                response->push_back(eHash);
                            }
                            else
                            {
                                ESP_LOGD(SYSTEM_DEBUG_HEADER, "error when convert hex string to uint8 array");
                            }
                        }
                    }
                }
            }

            break;
        }

    case CMD_OTA_WEBSOCKET:
        // server send websocket OTA request
        /*
            0 0xab, //websocket ota
            1 json.admin, //web client id
            2 segmentSize, //block size
            3 json.data.length, //total size
            4 arr[4], //time
            5 arr[5] //hash
        */
        {
            // check data
            if (output->size() == 6 &&
                output->at(OFFSET_START_OTA_ADMIN)->getType() == ETYPE_STRING &&
                output->at(OFFSET_START_OTA_HASH)->getType() == ETYPE_BUFFER)
            {
                // authorize user
                ESP_LOGD(SYSTEM_DEBUG_HEADER, "user authrozied");
                bool isAdmin = false;
                if (this->authorize(output->at(OFFSET_START_OTA_HASH), output->at(OFFSET_START_OTA_TIMESTAMP), &isAdmin))
                {
                    if (isAdmin) // only administrator has authority to do this action
                    {
                        // setup a hard reset when ota update failed for unknown reason
                        setTimeout(
                            []()
                            {
                                ESP.restart();
                            },
                            OTA_UPDATE_HARD_RESET_TIMEOUT);

                        // build ota websocket client object
                        this->ota = new WebsocketOTA(
                            output, // input arguments

                            // start failed callback
                            [this, response, output](int code)
                            {
                                ESP_LOGD(SYSTEM_DEBUG_HEADER, "OTA update start failed");
                                // 1 == board id, string
                                // 2 == web client id, string
                                // 3 == log, string
                                response->push_back(new Element(CMD_LOG));
                                response->push_back(new Element(this->getUniversalID().getRawBuffer(), 32));
                                response->push_back(new Element(output->at(1)->getString()));
                                response->push_back(new Element("ota start failed"));
                                delete this->ota;
                            },

                            [this](int code)
                            { delete this->ota; } // abort callback
                        );

                        // start ota update
                        this->ota->start(this->UniversalID,
                                         this->globalRemoteWebsocketDomain,
                                         this->globalRemoteWebsocketPort,
                                         this->globalRemoteWebsocketPath);

                        // give a response to administrator
                        // using log channel
                        response->push_back(new Element(CMD_LOG));
                        response->push_back(new Element(this->getUniversalID().getRawBuffer(), 32));
                        response->push_back(new Element(output->at(1)->getString()));
                        response->push_back(new Element("OTA Update Started"));

                        // record start time
                        global->otaStartTime = millis();

                        xTaskCreateUniversal(otaTask,
                                             "otaTask",
                                             OTA_TASK_DEFAULT_STACK_SIZE,
                                             NULL,
                                             OTA_TASK_DEFAULT_PRIOTITY,
                                             NULL,
                                             ARDUINO_RUNNING_CORE);
                    }
                }
            }
            else
            {
                ESP_LOGD(SYSTEM_DEBUG_HEADER, "Invalid arguments");
            }

            break;
        }

    case CMD_FIND_DEVICE:
        // find device
        // 1 == admin id
        // 2 == user name sha256
        // 3 == time uint64
        // 4 == hash uint8 array
        {
            bool isAdmin = false;
            if (lengthOfOutput == FIND_DEVICE_VECTOR_LENGTH &&
                output->at(4)->getType() == ETYPE_BUFFER &&
                this->authorize(output->at(4), output->at(3), &isAdmin))
            {
                // role authorized
                // send basic information
                // back command is 0xfa
                this->getFindDeviceBuffer(output->at(1)->getString().c_str(), response, isAdmin);

                if (msgShouldConfirm)
                {
                    uint64_t originalCmd = response->at(0)->getUint64();
                    originalCmd |= msgID; // attach msg id
                    originalCmd |= CMD_CONFIRM;
                    response->at(0)->setNumber(originalCmd, ETYPE_UINT64);
                }
            }

            break;
        }

    case CMD_EXECUTE_COMMAND:
        // execute command
        {
            this->executeCommand(output, response);
            if (msgShouldConfirm)
            {
                uint64_t originalCmd = response->at(0)->getUint64();
                originalCmd |= msgID; // attach msg id
                originalCmd |= CMD_CONFIRM;
                response->at(0)->setNumber(originalCmd, ETYPE_UINT64);
            }
            break;
        }

    default:

        if (this->extraWifiWebsocketCallback)
            this->extraWifiWebsocketCallback(this->websocketClient, event, data, length, output);
        break;
    }

    // create buffer and send it if response container isn't empty
    if (response->size())
    {
        // create buffer
        uint32_t outLen = 0;
        uint8_t *buffer = ArrayBuffer::createArrayBuffer(response, &outLen);

        if (buffer && outLen)
        {
            // send buffer
            this->websocketClient->send(buffer, outLen);

            // clear buffer
            delete buffer;
        }

        // delete elements
        for (std::vector<Element *>::iterator it = response->begin();
             it != response->end();
             ++it)
        {
            delete (*it);
        }
    }
    // delete container
    delete response;
}

bool GlobalManager::sendMessageToClient(const Element &msg)
{
    if (this->isWifiConnected && this->websocketClient && this->websocketClient->connected())
    {
        uint32_t outLen = 0;

        uint8_t *buffer = ArrayBuffer::createArrayBuffer(
            {this->eLogCommand, this->UniversalID, this->eAdminIDStr, msg}, &outLen);

        if (outLen && buffer)
        {
            this->websocketClient->send(buffer, outLen);
            delete buffer;
            Serial.println("message sent");
            return true;
        }
    }
    return false;
}

OneTimeAuthorization *GlobalManager::generateOneTimeAuthorization()
{
    OneTimeAuthorization *authorization = new OneTimeAuthorization();
    authorization->time = globalTime->getTime();
    char time[24] = {0};
    sprintf(time, "%llu", authorization->time);
    String hash = this->userName.getHex() + this->password.getHex() + time;
    authorization->hash = new (std::nothrow) uint8_t[32];
    mycrypto::SHA::sha256(hash.c_str(), authorization->hash);
    return authorization;
}

template <class T>
bool GlobalManager::_sendBundle(String frinedID, T command, uint16_t providerID, Elements *request)
{
    if (!this->websocketClient)
        return false;
    if (!this->websocketClient->connected())
        return false;

    if (!request)
    {
        // create container
        request = new std::vector<Element *>();
        if (std::is_same<T, uint8_t>::value)
        {
            // short command
            request->push_back(new Element((uint8_t)(command)));
        }
        else if (std::is_same<T, uint64_t>::value)
        {
            // long command
            request->push_back(new Element(this->patchLongCommandFixedHeader(command)));
        }
        else
        {
            return false;
        }

        // fill elements
        // command

        // friend id
        request->push_back(new Element(frinedID));

        // self id
        request->push_back(new Element(this->UniversalID.getHex()));

        // generate one time authorization data
        OneTimeAuthorization *authorize = this->generateOneTimeAuthorization();

        // fill authorize information
        request->push_back(new Element(authorize->time));
        request->push_back(new Element(authorize->hash, 32));

        // fill provider id
        request->push_back(new Element(providerID));

        // remove original information
        // the constructor of Element will make a copy
        delete authorize->hash;
        delete authorize;
    }

    // prepare generate buffer
    uint32_t outLen = 0;

    // generate buffer
    uint8_t *buffer = ArrayBuffer::createArrayBuffer(request, &outLen);

    // check buffer
    if (buffer && outLen)
    {
        // send buffer to remote server
        this->websocketClient->send(buffer, outLen);

        // remove buffer
        delete buffer;
    }

    // do clean process with container
    for (uint32_t i = 0; i < request->size(); ++i)
    {
        delete request->at(i);
    }
    return true;
}

void GlobalManager::buildRegisterBuffer()
{
    this->registerBuffer = ArrayBuffer::createArrayBuffer(
        {CMD_REGISTER_OR_ROLE_AUTHORIZE,          // command
         this->UniversalID,                       // esp32 id
         this->userName,                          // admin user name(sha256)
         (this->isNewFirmwareBoot ? 0x01 : 0x00), // if after ota updated
         ((this->usersBuffer &&
           this->lengthOfUsersBuffer)
              ? (new Element(this->usersBuffer, this->lengthOfUsersBuffer))
              : (new Element(0x00)))},
        &(this->lengthOfRegisterBuffer));
}

void GlobalManager::internalUniversalWebsocketCallback(myWebSocket::WebSocketClient *client,
                                                       myWebSocket::WebSocketEvents event,
                                                       uint8_t *data,
                                                       uint64_t length)
{
    if (event == myWebSocket::WS_CONNECTED)
    {
        ESP_LOGD(SYSTEM_DEBUG_HEADER, "%s", (client ? "Client online" : "Websocket connected"));

        this->serverOnline(1);

        // record connected time
        if (!this->remoteWebsocketConnectedTimestamp)
        {
            this->remoteWebsocketConnectedTimestamp = millis();
        }

        // register to server to fetch time
        uint8_t *tmpBuffer = new (std::nothrow) uint8_t[this->lengthOfRegisterBuffer];
        if (!tmpBuffer)
        {
            ESP_LOGD(SYSTEM_DEBUG_HEADER, "register buffer memory allocate failed");
            return;
        }
        memcpy(tmpBuffer, this->registerBuffer, this->lengthOfRegisterBuffer);
        this->websocketClient->send(tmpBuffer, this->lengthOfRegisterBuffer);
        delete tmpBuffer;
    }
    else if (event == myWebSocket::WS_DISCONNECTED || event == myWebSocket::TCP_ERROR)
    {
        // websocket disconnected
        ESP_LOGD(SYSTEM_DEBUG_HEADER, "%s", (client ? "client offline" : "server offline"));
        this->remoteWebsocketConnectedTimestamp = 0;
        this->serverOnline(-2);
    }
    else if (event == myWebSocket::TYPE_BIN)
    {
        this->routeDataToWebsocketHandler(data, length, client, event);
    }
}

void GlobalManager::loadWebsocketInformation()
{
    // read domain, port and path from database
    this->globalRemoteWebsocketDomain = db("websocketDomain")->getString();

#ifdef ENABLE_USE_PRESET_WEBSOCKET_INFORMATION
    if (!this->globalRemoteWebsocketDomain.length())
    {
#ifdef PRESET_WEBSOCKET_DOMAIN
        this->globalRemoteWebsocketDomain = String(PRESET_WEBSOCKET_DOMAIN);
#endif
    }
#endif

    ESP_LOGD(SYSTEM_DEBUG_HEADER, "websocket domain: %s\n", this->globalRemoteWebsocketDomain.c_str());

    if (db("websocketPort")->getType() == ETYPE_STRING)
    {
        (*db("websocketPort")) = (uint16_t)(db("websocketPort")->getString().toInt());
        db.flush();
    }

    this->globalRemoteWebsocketPort = db("websocketPort")->getUint16();

#ifdef ENABLE_USE_PRESET_WEBSOCKET_INFORMATION
    if (!this->globalRemoteWebsocketPort)
    {
#ifdef PRESET_WEBSOCKET_PORT
#if PRESET_WEBSOCKET_PORT > 0
        this->globalRemoteWebsocketPort = (uint16_t)(PRESET_WEBSOCKET_PORT);
#endif
#endif
    }
#endif

    ESP_LOGD(SYSTEM_DEBUG_HEADER, "websocket port: %lu\n", this->globalRemoteWebsocketPort);

    this->globalRemoteWebsocketPath = db("websocketPath")->getString();

#ifdef ENABLE_USE_PRESET_WEBSOCKET_INFORMATION
    if (!this->globalRemoteWebsocketPath.length())
    {
#ifdef PRESET_WEBSOCKET_PATH
        this->globalRemoteWebsocketPath = String(PRESET_WEBSOCKET_PATH);
#endif
    }
#endif

    ESP_LOGD(SYSTEM_DEBUG_HEADER, "websocket path: %s\n", this->globalRemoteWebsocketPath.c_str());

    // set websocket callback
    this->websocketClient->setCallBack(
        // this first layer of callback
        [](
            myWebSocket::WebSocketEvents event, // websocket event
            uint8_t *data,                      // payload
            uint64_t length                     // length of payload
        )
        {
            // call universal callback
            global->internalUniversalWebsocketCallback(nullptr, event, data, length);
        });
}

void GlobalManager::connectWebocket()
{
    if (!this->globalRemoteWebsocketPort)
    {
        this->loadWebsocketInformation();
    }

    ESP_LOGD(SYSTEM_DEBUG_HEADER, "Connecting to remote websocket server...");

    this->websocketClient->setAutoReconnect(true, 5000);

    // connect
    this->websocketClient->connect(
        this->globalRemoteWebsocketDomain,
        this->globalRemoteWebsocketPort,
        this->globalRemoteWebsocketPath);
}

void GlobalManager::initializeBasicInformation()
{
    // read user name and password(administrator)
    this->beginUserInfo();

    // build buffer if there not only administrator account exists
    this->buildUsersBuffer();

    // this buffer is for register to server
    // it will contains some information, like esp32 id, admin user name(sha256), and other user names
    if (this->userName.available() && this->password.available())
    {
        this->beginLogCommandAndAdminID();
        this->buildRegisterBuffer();
    }
    else
    {
// check preset settings
#ifdef PRESET_ADMIN_USERNAME
#ifdef PRESET_ADMIN_PASSWORD
        // preset information for administrator detected
        Element userNameOfAdmin(PRESET_ADMIN_USERNAME);
        Element userPasswordOfAdmin(PRESET_ADMIN_PASSWORD);

        if (userNameOfAdmin.getString().length() == 64 &&
            userPasswordOfAdmin.getString().length() == 64)
        {
            ESP_LOGD(SYSTEM_DEBUG_HEADER, "preset admin info loaded");
            if (userNameOfAdmin.convertHexStringIntoUint8Array() &&
                userPasswordOfAdmin.convertHexStringIntoUint8Array())
            {
                *db("userName") = userNameOfAdmin;
                *db("password") = userPasswordOfAdmin;
                db.flush();

                ESP_LOGD(SYSTEM_DEBUG_HEADER, "preset admin info stored");
                this->buildRegisterBuffer();
            }
        }
#endif
#endif
    }

    // read nickname from flash to ram
    this->nickname = db("nickname");

    if (!this->nickname.available())
    {
        // universal id instead of nickname if nickname empty
        this->nickname = this->UniversalID;
    }

    if (this->isWifiEnabled)
    {
        // mark if there wifi information in flash
        this->isWiFiInfoOK =
            db("wifiSSID")->getString().length() && db("wifiPwd")->getString().length();

        this->isWiFiInfoOK = this->isWiFiInfoOK ? 0xffu : 0;

#ifdef APP_REQUIRE_START_AP_ON_BOOT
        // start ap
        this->startAP();
#else
#ifdef ENABLE_DEALY_START_AP
        setTimeout(
            []()
            {
                // start ap
                global->globalTask |= GT_START_AP;
            },
            DELAY_START_AP_TIMEOUT);
#else
        // start ap
        this->startAP();
#endif
#endif
        if (this->isWiFiInfoOK && this->userName.available() && this->password.available())
        {
            // connect to wifi if wifi ssid and password detected in database
            ESP_LOGD(SYSTEM_DEBUG_HEADER, "wifi ssid and password detected in database");

            // connect wifi
            ESP_LOGD(SYSTEM_DEBUG_HEADER, "Connecting to wifi...");
            this->connectWifi();
        }
        else
        {
#ifdef PRESET_WIFI_SSID
#ifdef PRESET_WIFI_PASSWORD
            ESP_LOGD(SYSTEM_DEBUG_HEADER, "Connecting to wifi...");
            this->connectWifi();
#else
            ESP_LOGD(SYSTEM_DEBUG_HEADER, "No WiFi information detected in database");
#endif
#else
            ESP_LOGD(SYSTEM_DEBUG_HEADER, "No WiFi information detected in database");
#endif
        }

#ifdef PROACTIVE_DETECT_REMOTE_SERVER
        if (!this->tDetectServer)
        {
            this->tDetectServer = setInterval(
                []()
                {
                    global->globalTask |= GT_DETECT_REMOTE_SERVER;
                },
                DETECT_SERVER_ONLINE_INTERVAL);
        }
#endif
    }

#ifdef BUILT_IN_FUNCTION_PROVIDER

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            if (!arguments->size())
                return nullptr;

            Element *payload = arguments->at(0);

            Element *rtnValue = new Element("OK");

            auto typeOfPayload = payload->getType();
            if (typeOfPayload == ETYPE_VOID)
                return nullptr;

            if (typeOfPayload == ETYPE_STRING)
            {
                if (payload->getRawBufferLength() > 2)
                {
                    Serial.write(payload->c_str(), payload->getRawBufferLength() - 1);
                }
                else
                {
                    *rtnValue = "empty string";
                }
            }
            else if (typeOfPayload == ETYPE_BUFFER)
            {
                if (payload->getRawBufferLength())
                {
                    Serial.write(payload->getUint8Array(), payload->getRawBufferLength());
                }
                else
                {
                    *rtnValue = "empty buffer";
                }
            }
            else
            {
                *rtnValue = "type error";
            }

            return rtnValue;
        },
        PI_WEB_SERIAL, (PROVIDER_ADMIN | PROVIDER_COMMON | PROVIDER_ENCRYPT), 1);

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            esp_reset_reason_t reason = esp_reset_reason();
            String strReason = "";
            switch (reason)
            {
            case ESP_RST_UNKNOWN:
                strReason = PI_UNKNOWN;
                break;
            case ESP_RST_POWERON:
                strReason = PI_BOOT;
                break;
            case ESP_RST_INT_WDT:
                strReason = PI_ESP_RST_INT_WDT;
                break;
            case ESP_RST_TASK_WDT:
                strReason = PI_ESP_RST_TASK_WDT;
                break;
            case ESP_RST_WDT:
                strReason = PI_ESP_RST_WDT;
                break;
            case ESP_RST_PANIC:
                strReason = PI_ESP_RST_PANIC;
                break;
            case ESP_RST_BROWNOUT:
                strReason = PI_ESP_RST_BROWNOUT;
                break;
            case ESP_RST_DEEPSLEEP:
                strReason = PI_ESP_RST_DEEPSLEEP;
                break;
            case ESP_RST_SW:
                strReason = PI_ESP_RST_SW;
                break;
            default:
                strReason = PI_UNKNOWN;
            }

            char buffer[128] = {0};
            sprintf(buffer, "%s, %llu", strReason.c_str(), global->systemPowerOnTime);

            return new Element(buffer);
        },
        PI_GET_RESET_AND_ONLINE_TIME,
        (PROVIDER_ADMIN | PROVIDER_COMMON));

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            if (!arguments->size())
                return new Element(PI_INVALID_LENGTH_OF_ARGUMENTS);

            if ((arguments->at(0)->getType() != ETYPE_STRING) || (!arguments->at(0)->getString().length()))
                return new Element(PI_INVALID_ARGUMENT);

            *db("token") = arguments->at(0);
            db.flush();

            return new Element(PI_TOKEN_SET);
        },
        PI_MODIFY_TOKEN, (PROVIDER_ADMIN | PROVIDER_COMMON | PROVIDER_ENCRYPT), 1);

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            if (arguments->size() < 2)
            {
                return new Element(PI_INVALID_LENGTH_OF_ARGUMENTS);
            }

            if (arguments->at(0)->getType() != ETYPE_STRING || arguments->at(1)->getType() != ETYPE_STRING)
            {
                return new Element(PI_INVALID_TYPE_OF_ARGUMENT);
            }

            if ((!arguments->at(0)->getString().length()))
            {
                return new Element(PI_INVALID_LENGTH_OF_ARGUMENT);
            }

            uint8_t userNameHash[32] = {0};
            mycrypto::SHA::sha256((uint8_t *)arguments->at(0)->getString().c_str(),
                                  arguments->at(0)->getString().length(),
                                  userNameHash);
            Element *eUserName = new Element(userNameHash, 32);
            if (!arguments->at(1)->getString().length())
            {
                *(dbUser(eUserName)) = "";
                dbUser.flush();
                return new Element(PI_USER_DELETED);
            }
            uint8_t passwordHash[32] = {0};

            mycrypto::SHA::sha256((uint8_t *)arguments->at(1)->getString().c_str(),
                                  arguments->at(1)->getString().length(),
                                  passwordHash);

            Element *ePassword = new Element(passwordHash, 32);

            *(dbUser(eUserName)) = ePassword;
            dbUser.flush();

            return new Element(PI_SUCCESS);
        },
        PI_ADD_MODIFY_DELETE_USER, (PROVIDER_ADMIN | PROVIDER_COMMON | PROVIDER_ENCRYPT), 2);

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            if (arguments->size() < 2)
                return new Element(PI_INVALID_ARGUMENT);

            Element *key = arguments->at(0);
            Element *value = arguments->at(1);

            if (key->available())
            {
                if (value->available())
                {
                    // update datebase
                    *(db(arguments->at(0))) = arguments->at(1);
                    db.flush();
                    return new Element(PI_DATABASE_UPDATED);
                }
                else
                {
                    // query
                    // former heap that pointer "value" pointed to will be clean automatically
                    value = new Element();
                    value->copyFrom(db(arguments->at(0)));
                    return value;
                }
            }
            else
            {
                // list all keys
                String keys = "";
                std::vector<Unit *> *list = db.list();
                std::vector<Unit *>::iterator it = list->begin();
                std::vector<Unit *>::iterator end = list->end();
                ElementType type = ETYPE_VOID;

                for (; it != end; ++it)
                {
                    if ((*it)->key->available())
                    {
                        keys += (*it)->key->getString() + ", ";
                    }
                }
                keys = keys.substring(0, keys.length() - 2);
                return new Element(keys);
            }
        },
        PI_QUERY_UPDATE_MAIN_DATABASE_OR_LIST_ALL_KEYS_IN_MAIN_DATABASE,
        (PROVIDER_ADMIN | PROVIDER_ENCRYPT | PROVIDER_QUESTION), 2);

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            if (arguments->size() < 1)
                return new Element(PI_INVALID_LENGTH_OF_ARGUMENTS);

            if (!arguments->at(0)->available())
                return new Element(PI_INVALID_ARGUMENT);

            if (!arguments->at(0)->getString().length())
                return new Element(PI_INVALID_ARGUMENT);

            (*db(arguments->at(0))) = "";
            db.flush();

            return new Element(PI_DELETED);
        },
        PI_REMOVE_VALUE_IN_MAIN_DATABASE, (PROVIDER_ADMIN | PROVIDER_ENCRYPT | PROVIDER_QUESTION), 1);

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            global->resetWifiInfo();
            return new Element(PI_WIFI_INFORMATION_REMOVED);
        },
        PI_REMOVE_WIFI_INFORMATION, (PROVIDER_ADMIN | PROVIDER_QUESTION));

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            if (!esp_ota_check_rollback_is_possible())
            {
                return new Element(PI_CAN_NOT_ROLLBACK);
            }
            *(db("rebootReason")) = "rollback";
            db.flush();
            setTimeout([]()
                       { esp_ota_mark_app_invalid_rollback_and_reboot(); },
                       3000);
            return new Element(PI_WILL_ROLLBACK_IN_TIMEOUT_SECONDS);
        },
        PI_ROLLBACK, (PROVIDER_ADMIN | PROVIDER_COMMON | PROVIDER_QUESTION));

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            String rtnValue = "";
            Element pendingOTAInvalid = db("pendingOTAInvalid");
            Element pendingOTA = db("pendingOTA");

            if (pendingOTAInvalid.available())
            {
                uint64_t t = pendingOTAInvalid.getNumber();
                if (t)
                {
                    // return error time
                    char buf[64] = {0};
                    sprintf(buf, PI_FIRMWARE_ERROR_TIME, t);
                    return new Element(buf);
                }

                rtnValue = pendingOTA.available()
                               ? (pendingOTA.getUint8()
                                      ? (PI_FIRMWARE_PENDING_VERIFY)
                                      : (PI_FIRMWARE_VALID))
                               : (PI_NO_OTA_INFORMATION);
            }
            else
            {
                rtnValue = PI_NO_OTA_INFORMATION;
            }
            return new Element(rtnValue);
        },
        PI_FIRMWARE_STATUS, (PROVIDER_ADMIN | PROVIDER_COMMON));

    this->createProvider(
        true,
        [this](ProviderArguments arguments) -> Element *
        {
            Element lastOTATime = db("lastOTATime");
            if (lastOTATime.available())
                return new Element(lastOTATime.getNumber());

            return new Element(0);
        },
        PI_LAST_OTA_TIME, (PROVIDER_ADMIN | PROVIDER_COMMON));

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            uint8_t mac[8] = {0};
            esp_efuse_mac_get_default(mac);
            String unformatted = Element(mac, 8).getHex(false);
            String formatted = "";
            for (uint32_t i = 0; i < unformatted.length() - 4; i += 2)
            {
                formatted += unformatted.substring(i, i + 2) + ":";
            }
            formatted = formatted.substring(0, formatted.length() - 1);
            return new Element(formatted);
        },
        PI_MAC_ADDRESS, (PROVIDER_ADMIN | PROVIDER_COMMON | PROVIDER_ENCRYPT));

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            char rtnValue[64] = {0};
            if (!arguments->size())
            {
                setTimeout(
                    []()
                    {
                        *(db("rebootReason")) = "deep sleep";
                        db.flush();
                        ESP.deepSleep(600 * 1000 * 1000);
                    },
                    3000);
                sprintf(rtnValue, PI_WILL_INTO_DEEP_SLEEP_FOR_TIMEOUT_SECONDS_IN_SECONDS, 600, 3);
                return new Element(rtnValue);
            }
            else
            {
                if (arguments->at(0)->getType(true) == ETYPE_NUMBER)
                {
                    uint32_t t = arguments->at(0)->getNumber();
                    setTimeout(
                        [t]()
                        {
                            *(db("rebootReason")) = "deep sleep";
                            db.flush();
                            ESP.deepSleep(t * 1000 * 1000);
                        },
                        3000);

                    sprintf(rtnValue, PI_WILL_INTO_DEEP_SLEEP_FOR_TIMEOUT_SECONDS_IN_SECONDS, 3, t);
                    return new Element(rtnValue);
                }
                else
                {
                    return new Element(PI_INVALID_ARGUMENT);
                }
            }
        },
        PI_DEEP_SLEEP, (PROVIDER_ADMIN | PROVIDER_COMMON | PROVIDER_QUESTION), 1);

    this->createProvider(
        [](ProviderArguments arguments) -> Element *
        {
            MyFS::formatSPIFFS();
            global->delayReboot();
            return new Element(PI_ALL_INFORMATION_REMOVED_REBOOT_IN_3_SECONDS);
        },
        PI_REMOVE_ALL_INFORMATION, (PROVIDER_ADMIN | PROVIDER_QUESTION));

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            Element *value = new Element(PI_INVALID_SSID_PROVIDED);
            if (!arguments->size())
                return value;
            if (!arguments->at(0)->getType() == ETYPE_STRING || !arguments->at(0)->getString().length())
                return value;

            delete value;

            *(db("wifiSSID")) = arguments->at(0)->getString();
            db.flush();

            char a[64]{0};
            sprintf(a, PI_NEW_SSID_OR_PASSWORD_HAS_BEEN_SET, "SSID", arguments->at(0)->getString().c_str());

            return new Element(a);
        },
        PI_MODIFY_SSID, (PROVIDER_ADMIN | PROVIDER_ENCRYPT | PROVIDER_QUESTION), 1);

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            Element *value = new Element(PI_INVALID_WIFI_PASSWORD);
            if (!arguments->size())
                return value;
            if (!arguments->at(0)->getType() == ETYPE_STRING || !arguments->at(0)->getString().length())
                return value;
            if (arguments->at(0)->getString().length() < 8)
                return value;

            delete value;

            *(db("wifiPwd")) = arguments->at(0)->getString();
            db.flush();

            char a[64] = {0};
#ifdef ENGLISH_VERSION
            sprintf(a, PI_NEW_SSID_OR_PASSWORD_HAS_BEEN_SET, " password", arguments->at(0)->getString().c_str());
#else
            sprintf(a, PI_NEW_SSID_OR_PASSWORD_HAS_BEEN_SET, "密码", arguments->at(0)->getString().c_str());
#endif

            return new Element(a);
        },
        PI_MODIFY_WIFI_PASSWORD, (PROVIDER_ADMIN | PROVIDER_ENCRYPT | PROVIDER_QUESTION), 1);

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        {
            uint32_t delay = 3;
            if (arguments->size())
            {
                if (arguments->at(0)->getNumber())
                {
                    delay = arguments->at(0)->getNumber();
                }
            }
            *(db)("rebootReason") = "remote manual";
            db.flush();
            global->delayReboot(delay * 1000);
            char a[64] = {0};
            sprintf(a, PI_WILL_REBOOT_IN_TIMEOUT_SECONDS, delay);
            return new Element(a);
        },
        PI_REBOOT, (PROVIDER_ADMIN | PROVIDER_COMMON | PROVIDER_QUESTION), 1);

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        { return new Element(globalTime->getTime()); },
        PI_HARDWARE_TIMESTAMP, (PROVIDER_ADMIN | PROVIDER_COMMON));

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        { return new Element(String(ESP.getFreeHeap()) + PI_PREFIX_BYTE); },
        PI_FREE_HEAP, (PROVIDER_ADMIN | PROVIDER_COMMON));

    this->createProvider(
        true,
        [](ProviderArguments arguments) -> Element *
        { return new Element(String(MyFS::getFreeSpace()) + PI_PREFIX_BYTE); },
        PI_FREE_SPACE, (PROVIDER_ADMIN | PROVIDER_COMMON));

#endif

#ifdef ARDUINO_RUNNING_CORE
    xTaskCreateUniversal(APScheduler,
                         "APScheduler",
                         1024,
                         NULL,
                         1,
                         NULL,
                         ARDUINO_RUNNING_CORE);
#else
    xTaskCreateUniversal(APScheduler,
                         "APScheduler",
                         1024,
                         NULL,
                         1,
                         NULL,
                         0);
#endif
}

void GlobalManager::setSerialRecvCb()
{
    Serial.onReceive(
        [this]()
        {
            if (this->isSerialDataLoopBack && this->isWifiEnabled && this->isWifiConnected)
            {
                size_t size = Serial.available();

                if (!size)
                {
                    return;
                }

                if (size <= 126)
                {
                    char buf[128] = {0};

                    size = Serial.readBytes(buf, Serial.available());
                    this->sendMessageToClient(Element((uint8_t *)buf, size, false, 0));
                }
                else
                {
                    if (size >= 81916)
                    {
                        ESP_LOGD(SYSTEM_DEBUG_HEADER, "long length");
                        return;
                    }
                    size += 4;
                    char *buf = new (std::nothrow) char[size];
                    if (!buf)
                    {
                        ESP_LOGD(SYSTEM_DEBUG_HEADER, "memory allocate failed");
                        return;
                    }
                    bzero(buf, size);
                    size = Serial.readBytes(buf, size - 4);
                    this->sendMessageToClient(Element((uint8_t *)buf, size, false, 0));
                    delete buf;
                }
            }
        });
}

void GlobalManager::loop()
{
    app->loop();

    if (this->globalTask)
    {
        if (this->globalTask & GT_START_AP)
        {
            this->startAP();
        }

        if (this->globalTask & GT_DETECT_REMOTE_SERVER)
        {
            if (
                !this->isOTAUpdateRunning() &&
                this->wifiConnected())
            {
                uint16_t times = this->serverOfflineTimes();

                // start AP if maximum value exceeded
                if (times > AUTOMATIC_START_AP_IF_REMOTE_WEBSOCKET_DISCONNECTED_TIMES &&
                    !this->isAPStarted)
                {
                    // start ap
                    this->globalTask |= GT_START_AP;
                }

#ifdef REBOOT_IF_PROACTIVE_DETECT_REMOTE_SERVER_OFFLINE
                if (times > REBOOT_IF_PROACTIVE_DETECT_REMOTE_SERVER_OFFLINE_TIMES)
                {
                    ESP_LOGD(SYSTEM_DEBUG_HEADER, "couldn't connect to remote server, reboot");
                    ESP.restart();
                }
#endif

                // mark server is offline
                this->serverOnline(-2);

                // send hello to server then wait for response from server
                this->sendHello();

                ESP_LOGD(SYSTEM_DEBUG_HEADER, "hello sent");
            }
        }

        if (this->globalTask & GT_CLOSE_AP)
        {
            this->closeAP();
        }
        this->globalTask = 0ULL;
    }

    if (this->isWifiEnabled)
    {
        if (this->isWifiConnected)
        {
            // loop remote websockets
            this->websocketClient->loop();

            if (this->isNewFirmwareBoot)
            {
                auto t = millis();
                if (t - this->remoteWebsocketConnectedTimestamp > CONFIRM_NEW_FIRMWARE_VALID_TIMEOUT)
                {
                    this->markNewFirmwareIsValid();
                    ESP_LOGD(SYSTEM_DEBUG_HEADER, "New firmware is valid");
                }
            }
        }
        else
        {
            if (this->isWiFiInfoOK)
            {
                auto t = millis();
                if (t - this->lastConnectWiFiTime > AUTO_CONNECT_WIFI_TIMEOUT)
                {
                    this->lastConnectWiFiTime = t;
                    this->connectWifi();
                }
            }
        }
    }

    // loop ap server
    if (this->isAPStarted && this->apServer)
    {
        this->apServer->loop();
    }

    // dnsServer->processNextRequest();
    // yield();
}

void GlobalManager::getFindDeviceBuffer(
    const char *userID,
    Elements *response,
    bool isAdmin)
{
    /*
        C code:
        for find device
        arr[0] 0xfa
        arr[1] admin id, string
        arr[2] current cpu freq, MHz, uint16
        arr[3] uint8
                0                                   0                                   0 0 0 0 0 0
                network mode                                                            extra
                wifi                                ap
        arr[4] device timestamp, uint64
        arr[5] free heap in bytes, uint32
        arr[6] nickname, string
        arr[7] universal id(board id)
  */

    uint8_t extraInfo = 0;
    /*
        0       0       0       0   0   0   0   0
        wifi    ap      admin   extra
    */
    if (this->isWifiConnected)
    {
        extraInfo |= (uint8_t)(128);
    }
    if (this->isAPStarted)
    {
        extraInfo |= (uint8_t)(64);
    }
    if (isAdmin)
    {
        if (this->isProviderBufferShrank)
        {
            this->buildProvidersBuffer();
        }

        // record admin online time
        this->lastTimeAdminOnline = globalTime->getTime();

        // set mask for js use
        extraInfo |= (uint8_t)(32);
    }

    response->push_back(new Element(CMD_FIND_DEVICE_RESPONSE));                            // response to find device 0xaf
    response->push_back(new Element(userID));                                              // web client id
    response->push_back(new Element((uint16_t)(ESP.getCpuFreqMHz())));                     // cpu freq
    response->push_back(new Element(extraInfo));                                           // extra info
    response->push_back(new Element(globalTime->getTime()));                               // current timestamp
    response->push_back(new Element((uint32_t)(ESP.getFreeHeap())));                       // free heap
    response->push_back(new Element(this->nickname.getString().c_str()));                  // nickname of this board
    response->push_back(new Element(this->UniversalID.getHex().c_str()));                 // id of this board
    response->push_back(new Element(SYSTEM_VERSION));                                      // current structure version
    response->push_back(new Element(String(APP_VERSION) + String(FIRMWARE_COMPILE_TIME))); // app version & compile time
    response->push_back(new Element(this->bufferProviders, this->bufferProvidersLength));  // providers buffer
}

void GlobalManager::resetWifiInfo()
{
    *(db("wifiSSID")) = "";
    *(db("wifiPwd")) = "";
    db.flush();
}

void GlobalManager::removeEarliestHash()
{
    if (this->usedHashes.size())
    {
        std::map<uint64_t, Element *>::iterator it = this->usedHashes.begin();
        if (globalTime->getTime() - it->first > ILLEGAL_TIMESPAN_TIMEOUT)
        {
            // remove earliest hash
            delete it->second;
            this->usedHashes.erase(it);

            // clear ram
            std::map<uint64_t, Element *>().swap(this->usedHashes);
        }
    }
}

bool GlobalManager::authorize(
    Element *remoteHash,
    Element *timestamp,
    bool *isAdmin)
{

    // if pointer is null, return false
    if (!timestamp || !remoteHash)
        return false;

    if (this->usedHashes.size() > MAXIMUM_USED_HASH_CAPACITY)
    {
        // reject all request if current device
        // is under attach
        // 当 当前设备被攻击时拒绝所有认证请求
        return false;
    }

    // current device not initialized, return false
    // 当 设备没有配置基础信息时直接拒绝请求
    if (!this->userName.available() || !this->password.available())
        return false;

    // reject request if invalid data type detected
    // 检测到不合法数据类型时直接拒绝认证请求
    if (remoteHash->getType() != ETYPE_BUFFER)
        return false;

    // check length of data
    // 检查数据长度
    if (remoteHash->getRawBufferLength() != SHA_LENGTH)
        return false;

    // check timestamp
    // a valid timestamp must be a 64bits number
    // 检查时间戳
    // 合格的时间戳必然是64位整数
    if (timestamp->getType() != ETYPE_UINT64)
        return false;

    // get time
    // 获取时间戳
    uint64_t t = timestamp->getUint64();

    // if time is zero return false
    // 时间戳为0 直接拒绝请求
    if (!t)
        return false;

    // prevent using same hash twice
    // 防止同一个一次性密匙使用多次
    if (this->usedHashes.find(t) != this->usedHashes.end())
        return false;

    // btw, remove eraliest hash
    // 顺便把最早用过的密匙删掉
    this->removeEarliestHash();

    // convert digital time into string
    // 将时间戳格式化为字符串
    char tmp[24] = {0};
    sprintf(tmp, "%llu", t);
    String time(tmp);

    // combine user name(sha256 hex string) and password(sha256 hex string) and time(string)
    // 组合字段
    String hash = this->userName.getHex() + this->password.getHex() + time;

    // container for sha result
    // 用于存放数字摘要的容器
    uint8_t localHash[SHA_LENGTH];

#ifdef ENABLE_SHA1_AUTHORIZATION
    // calc sha1
    // 计算sha1
    mycrypto::SHA::sha1((uint8_t *)hash.c_str(), hash.length(), localHash);
#else
    // calc sha256
    // 计算sha256
    mycrypto::SHA::sha256((uint8_t *)hash.c_str(), hash.length(), localHash);
#endif

    // get pointer of remote hash
    // 获取远程hash的指针便于操作
    uint8_t *buf = remoteHash->getUint8Array();

    // get local time
    // 获取本地时间
    uint64_t localTime = globalTime->getTime();

    // use non-copy mode
    // 使用非拷贝模式
    Element x(buf, SHA_LENGTH, false);
    Element y(localHash, SHA_LENGTH, false);

    if (x == y)
    {
        ESP_LOGD(SYSTEM_DEBUG_HEADER, "authorized as admin");
        // search admin
        (*isAdmin) = true;
        this->usedHashes.insert({localTime, new Element(localHash, 32)});
        return true;
    }
    else
    {
        ESP_LOGD(SYSTEM_DEBUG_HEADER, "search other users");
        // search other users
        if (dbUser.count() <= 0)
        {
            return false;
        }
        std::vector<Unit *> *users = dbUser.list();

        for (
            std::vector<Unit *>::iterator it = users->begin();
            it != users->end();
            ++it)
        {
            hash = (*it)->key->getHex() + (*it)->value->getHex() + time;

#ifdef ENABLE_SHA1_AUTHORIZATION
            // calc sha1
            mycrypto::SHA::sha1((uint8_t *)hash.c_str(), hash.length(), localHash);
#else
            // calc sha256
            mycrypto::SHA::sha256((uint8_t *)hash.c_str(), hash.length(), localHash);
#endif
            Element x(buf, SHA_LENGTH, false);
            Element y(localHash, SHA_LENGTH, false);
            if (x == y)
            {
                (*isAdmin) = false;
                this->usedHashes.insert({localTime, new Element(localHash, 32)});
                return true;
            }
        }
    }
    ESP_LOGD(SYSTEM_DEBUG_HEADER, "unauthorized");
    return false;
}

GlobalManager *global = new GlobalManager(true);