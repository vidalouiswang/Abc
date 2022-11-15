#include "mywebsocket.h"

namespace myWebSocket
{
    String generateServerKey(String clientKey)
    {
        // length of SHA1
        // SHA1长度
        int shaLen = 20;

        // container for sha1 result
        // 用于存放sha1结果的容器
        uint8_t output[shaLen] = {0};

        // this part use ESP32 SHA hardware acceleration module directly
        // you could change it if you like
        // 这部分直接使用ESP32 SHA加速器完成，你可以自己修改成其他SHA1方法
        mycrypto::SHA::sha1((uint8_t *)clientKey.c_str(), clientKey.length(), output);

        // temporary array for check sha1 result
        // the array output will be all zero if didn't enable SHA acceleration
        // 定义一个临时数组用于检测sha1的结果，如果没有提前使能SHA加速器，output会得到全0结果
        uint8_t tmp[shaLen] = {0};

        if (!strncmp((const char *)tmp, (const char *)output, shaLen))
        {
            // return empty string if sha got incorrent result
            // 如果SHA运算结果不正确返回空字符串
            return String("");
        }
        // else return the result with base64 encoded
        // 否则返回base64编码后的结果

        return mycrypto::Base64::base64Encode((const char *)output);
    }

    String WebSocketClient::generateHanshake()
    {
        // make a copy with websocket handshake template
        // 拷贝一份websocket握手头模板
        String wsHeader = String(myWebSocketHeader);

        // combine host(domain or ip) and port
        // 组合主机(域名或ip)和端口
        this->domain = String(this->host + ":" + String(this->port));

        // replace host and path
        wsHeader.replace("@HOST@", this->domain);
        wsHeader.replace("@PATH@", this->path);

        // generate 16 bytes random key
        uint8_t key[16];
        for (uint8_t i = 0; i < 16; i++)
        {
            key[i] = random(0xFF);
        }

        // hash
        uint8_t output[20];

        mycrypto::SHA::sha1(key, 16, output);

#ifdef DEBUG_MYCRYPTO
        bool isSHAValid = false;
        for (int i = 0; i < 16; i++)
        {
            if (output[i])
            {
                isSHAValid = true;
                break;
            }
        }

        if (!isSHAValid)
        {
            ESP_LOGW(MY_WEBSOCKET_DEBUG_HEADER, "SHA failed, SHA module may not enabled first");
            return String("");
        }
#endif

        char *a = mycrypto::Base64::base64Encode(output, 16);

        // store client key
        this->clientKey = String(a); // arduino String will make a copy
        delete a;

        wsHeader.replace("@KEY@", this->clientKey);
        return wsHeader;
    }

    bool WebSocketClient::connect(String url)
    {
        if (url.startsWith("wss://"))
        {
            return false;
        }
        if (url.startsWith("ws://"))
        {
            url = url.substring(url.indexOf("ws://") + 5);
        }

        int a = url.indexOf(":");
        int b = url.indexOf("/", (a < 0 ? 0 : a));

        String domain = "";
        uint16_t port = 80;
        String path = "/";

        if (a < 0)
        {
            if (b < 0)
            { // abc.com
                domain = url;
            }
            else
            { // abc.com/path
                domain = url.substring(0, b);
                path = url.substring(b);
            }
        }
        else
        {
            if (b < 0)
            { // abc.com:8080
                port = url.substring(a + 1).toInt();
            }
            else
            { // abc.com:8080/path
                port = url.substring(a + 1, b).toInt();
                path = url.substring(b);
            }
            domain = url.substring(0, a);
        }
        return this->connect(domain, port, path);
    }

    bool WebSocketClient::handShake()
    {
        String header = generateHanshake();
        if (header.isEmpty())
        {
            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "request header is empty");
            return false;
        }
        // generate server key to verify after server responsed
        String serverKey = this->clientKey + String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        serverKey = generateServerKey(serverKey);

        if (!serverKey.length())
        {
            if (this->fn)
            {
                this->fn(ERROR_GENERATE_KEY, nullptr, 0);
                return false;
            }
        }

        this->client->setTimeout(3);

        // connect to remote server
        if (!this->client->connect(this->host.c_str(), this->port))
        {
            // connect failed
            this->status = TCP_FAILED;
            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "connect failed");
            if (this->fn)
                this->fn(TCP_FAILED, nullptr, 0);
            return false;
        }
        else
        {
            // connected to remote server
            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "connected");
            this->client->setNoDelay(1);
            this->status = TCP_CONNECTED;

            // write handshake header
            uint64_t wroteLen = this->client->print(header);
            this->client->flush();

            if (wroteLen != header.length())
            {
                this->status = TCP_ERROR;
                ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "error when send handshake header to server");
                if (this->fn)
                    this->fn(WS_DISCONNECTED, nullptr, 0);
                return false;
            }

            while (!this->client->available())
            {
                yield();
            }

            // read server response
            uint64_t len = this->client->read(this->buffer, MY_WEBSOCKET_BUFFER_LENGTH);
            if (len > 0)
            {
                // add string end
                this->buffer[len] = 0;

                // convert to arduino String is more convenient
                String handShakeStr = String((char *)this->buffer);

                // check if there has and tail
                if (handShakeStr.indexOf("\r\n\r\n") >= 0)
                {
                    // get the server key
                    int keyStart = handShakeStr.indexOf("Sec-WebSocket-Accept: ") + 22;
                    int keyEnd = handShakeStr.indexOf("\r\n", keyStart);
                    String key = handShakeStr.substring(keyStart, keyEnd);
                    if (key == serverKey &&
                        handShakeStr.indexOf("Upgrade: websocket") >= 0 &&
                        handShakeStr.indexOf("Connection: Upgrade") >= 0 &&
                        handShakeStr.indexOf("HTTP/1.1 101") >= 0)
                    {
                        bzero(this->buffer, MY_WEBSOCKET_BUFFER_LENGTH);
                        this->status = WS_CONNECTED;
                        ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "websocket client connected");
                        if (this->fn)
                            this->fn(WS_CONNECTED, nullptr, 0);
                        return true;
                    }
                    else
                    {
                        // according to standard should disconnect socket
                        this->client->stop();
                        this->status = HANDSHAKE_UNKNOWN_ERROR;
                        ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "handshake failed, response header:(%s)", this->buffer);
                        if (this->fn)
                            this->fn(HANDSHAKE_UNKNOWN_ERROR, nullptr, 0);
                        return false;
                    }
                }
                else
                {
                    this->client->stop();
                    ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "handshake failed, response header:(%s)", this->buffer);
                    this->status = HANDSHAKE_UNKNOWN_ERROR;
                    if (this->fn)
                        this->fn(HANDSHAKE_UNKNOWN_ERROR, nullptr, 0);
                    return false;
                }
            }
            else
            {
                this->status = TCP_ERROR;
                if (this->fn)
                    this->fn(TCP_ERROR, nullptr, 0);
                return false;
            }
        }
    }

    uint64_t WebSocketClient::send(const char *data)
    {
        uint64_t len = strlen(data);
        uint8_t *d = (uint8_t *)malloc(len);
        if (!d)
        {
            return 0;
        }
        memcpy(d, data, len);
        uint64_t r = this->_send(TYPE_TEXT, d, strlen(data));
        free(d);
        return r;
    }

    uint64_t WebSocketClient::_send(WebSocketEvents type, uint8_t *data, uint64_t len)
    {
        if (!data || !len)
        {
            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "empty content to send");
            return 0;
        }

        uint8_t mask[4];
        for (int i = 0; i < 4; i++)
        {
            mask[i] = random(0xff);
        }

        // frame header
        uint8_t header[10];
        bzero(header, 10);

        // fin
        header[0] = header[0] | (uint8_t)128;

        // mask
        header[1] = this->isFromServer ? 0 : ((uint8_t)128);

        // fill frame type
        switch (type)
        {
        case TYPE_TEXT:
            header[0] |= (uint8_t)1;
            break;
        case TYPE_BIN:
            header[0] |= (uint8_t)2;
            break;
        case TYPE_CLOSE:
            header[0] |= (uint8_t)8;
            break;
        case TYPE_PING:
            header[0] |= (uint8_t)9;
            break;
        case TYPE_PONG:
            header[0] |= (uint8_t)10;
            break;
        default:
            return 0;
        }

        // convert length and send it to server
        if (len < 126)
        {
            header[1] = header[1] | (uint8_t)len;
            this->client->write((const char *)header, 2);
        }
        else if (len > 125 && len < 65536)
        {
            header[1] = header[1] | (uint8_t)126;
            uint16_t msgLen = (uint16_t)len;
            header[2] = (uint8_t)(msgLen >> 8);
            header[3] = (uint8_t)((msgLen << 8) >> 8);
            this->client->write((const char *)header, 4);
        }
        else
        {
            header[1] = header[1] | (uint8_t)127;
            uint64_t msgLen = len;
            for (int i = 0, j = 2; i < 8; i++, j++)
            {
                header[j] = (uint8_t)(((msgLen) << (i * 8)) >> 56);
            }
            this->client->write((const char *)header, 10);
        }

        // masking
        if (!this->isFromServer)
        {
            for (uint64_t i = 0; i < len; i++)
            {
                yield();
                data[i] = data[i] ^ mask[i & 3];
                yield();
            }
        }

        // send mask byte
        if (!this->isFromServer)
        {
            this->client->write((const char *)mask, 4);
        }

        // send data
        return this->client->write((const char *)data, len);
    }

    void WebSocketClient::loop()
    {

        if (this->status == WS_CONNECTED)
        {
            if (!this->client->connected())
            {
                ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "connection lost");

                // maual disconnect
                this->client->stop();

                // set statue
                this->status = WS_DISCONNECTED;

                // call callback
                if (this->fn)
                    this->fn(WS_DISCONNECTED, nullptr, 0);

                return;
            }

            // read frame header
            uint64_t len = this->client->read(this->buffer, 2);

            if (!len) // no msg
            {
                return;
            }

            if (len < 0) // socket error
            {
                this->client->stop();

                // set statue and call callback
                this->status = TCP_ERROR;
                if (this->fn)
                    this->fn(TCP_ERROR, nullptr, 0);

                return;
            }

            // msg arived
            bool isThisFrameisFin = this->buffer[0] & (uint8_t)128;

            // last frame
            WebSocketEvents type;
            uint8_t opcode = this->buffer[0] & (uint8_t)15;
            switch (opcode)
            {
            case 0:
                type = TYPE_CON;
                break;
            case 1:
                type = TYPE_TEXT;
                break;
            case 2:
                type = TYPE_BIN;
                break;
            case 9:
                type = TYPE_PING;
                break;
            case 10:
                type = TYPE_PONG;
                break;
            case 8:
                type = TYPE_CLOSE;
            default:
                type = TYPE_UNKNOWN;
                this->client->stop();
                this->status = WS_DISCONNECTED;
                if (this->fn)
                    this->fn(WS_DISCONNECTED, nullptr, 0);
                return;
            }

            // get real length type
            uint64_t length = (uint64_t)(this->buffer[1] & (uint8_t)(127));

            // marked if recv buffer has been deleted by user
            this->isRecvBufferHasBeenDeleted = false;

            // declare buffer pointer
            uint8_t *buf = nullptr;
            uint8_t maskBytes[4];

            // read real length
            if (length > 125)
            {
                // read real length
                bzero(this->buffer, MY_WEBSOCKET_BUFFER_LENGTH);

                // 126: payload length > 125 && payload length < 65536
                // 127: > 65535
                uint8_t extraPayloadBytes = length == 126 ? 2 : 8;
                length = 0;

                // read real length bytes
                this->client->read(this->buffer, extraPayloadBytes);

                extraPayloadBytes -= 1;

                // convert to uint64_t
                // for front byte
                for (uint8_t i = 0; i < extraPayloadBytes; i++)
                {
                    length += this->buffer[i];
                    length = length << 8;
                }
                // for last byte
                length += this->buffer[extraPayloadBytes];

                // see if beyond max length
                if (extraPayloadBytes > MY_WEBSOCKET_CLIENT_MAX_PAYLOAD_LENGTH)
                {
                    this->status = MAX_PAYLOAD_EXCEED;
                    if (this->fn)
                        this->fn(MAX_PAYLOAD_EXCEED, nullptr, 0);
                    ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "MAX_PAYLOAD_EXCEED");
                    return;
                }
            }

            // define another length for optimze unmask process
            uint64_t bufferLength = length;

            // length +1 for text type end of string
            if (type == TYPE_TEXT && isThisFrameisFin)
            {
                // for '\0' if there isn't '\0' at tail
                bufferLength += 1;
            }

            // for optimize unmask process
            bufferLength += 4 - (bufferLength % 4);
            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "optimized length:%d", bufferLength);

            // if length overflow it will be 0(though it won't happen forever on esp32 because of limited RAM size)
            if (!bufferLength || this->accBufferOffset + length > MY_WEBSOCKET_CLIENT_MAX_PAYLOAD_LENGTH)
            {
                this->status = MAX_PAYLOAD_EXCEED;
                if (this->fn)
                    this->fn(MAX_PAYLOAD_EXCEED, nullptr, 0);
                ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "MAX_PAYLOAD_EXCEED");
                return;
            }

            // allocate buffer
            buf = new (std::nothrow) uint8_t[bufferLength];

            // check buffer allocate success or not
            if (buf == nullptr)
            {
                this->status = BUFFER_ALLOC_FAILED;
                if (this->fn)
                    this->fn(BUFFER_ALLOC_FAILED, nullptr, 0);
                return;
            }

            // if this client is transfer from server
            // that means 4 bytes mask key should read at first
            if (this->isFromServer)
            {
                this->client->read(maskBytes, 4);
            }

            // otherwise this client is directly connected to remote
            // no mask key should read
            uint64_t readLength = this->client->read(buf, length);

            // read times
            int times = 0;

            // single read times
            int segmentLength = 0;

            // read data
            while (readLength < length && ++times < READ_MAX_TIMES)
            {
                while (!this->client->available())
                {
                    yield();
                    delay(1);
                }

                // read more data
                segmentLength = this->client->read(buf + readLength, length - readLength);

                // no more data to read
                if (!segmentLength)
                {
                    break;
                }

                // accumulate read length
                readLength += segmentLength;

                // reset single time read length
                segmentLength = 0;

                if (times > READ_MAX_TIMES)
                {
                    if (buf)
                        delete buf;

                    this->client->stop();
                    this->status = REACH_MAX_READ_TIMES;

                    if (this->fn)
                        this->fn(REACH_MAX_READ_TIMES, nullptr, 0);

                    return;
                }
            }

            if (readLength == length)
            {
                // correct data length
                if (this->isFromServer)
                {
                    // unmask
                    // this will consume 2200us if length type is uint64_t with 240MHz cpu config
                    // data size 64KB, AP mode
                    for (uint64_t i = 0; i ^ bufferLength; i += 4)
                    {
                        buf[i] = buf[i] ^ maskBytes[i & 3];
                        buf[(i + 1)] = buf[(i + 1)] ^ maskBytes[(i + 1) & 3];
                        buf[(i + 2)] = buf[(i + 2)] ^ maskBytes[(i + 2) & 3];
                        buf[(i + 3)] = buf[(i + 3)] ^ maskBytes[(i + 3) & 3];
                    }

                    // set extra tail zero
                    memset(buf + length, 0, bufferLength - length);
                }

                // copy data to extra buffer if this frame isn't last frame
                // otherwise call callback
                if (isThisFrameisFin)
                {
                    // call handler
                    if (this->accBufferOffset)
                    {
                        // copy last chunk
                        memcpy(this->accBuffer + this->accBufferOffset, buf, length);
                        this->accBufferOffset += length;

                        if (this->fn)
                            this->fn(type, this->accBuffer, this->accBufferOffset);

                        if (!this->isRecvBufferHasBeenDeleted)
                        {
                            delete this->accBuffer;
                        }
                        this->accBufferOffset = 0;
                    }
                    else
                    {
                        if (this->fn)
                            this->fn(type, buf, length);
                    }
                }
                else
                {
                    // define extra buffer
                    if (!this->accBufferOffset)
                    {
                        this->accBuffer = new (std::nothrow) uint8_t[MY_WEBSOCKET_CLIENT_MAX_PAYLOAD_LENGTH];

                        // check extra buffer allocate state
                        if (!(this->accBuffer))
                        {
                            if (buf != nullptr)
                            {
                                delete buf;
                            }

                            this->status = BUFFER_ALLOC_FAILED;
                            if (this->fn)
                                this->fn(BUFFER_ALLOC_FAILED, nullptr, 0);

                            return;
                        }
                    }

                    // copy original buffer
                    memcpy(this->accBuffer + this->accBufferOffset, buf, length);

                    // accumulate extra buffer offset
                    this->accBufferOffset += length;
                }
            }
            else
            {
                this->client->stop();
                this->status = TCP_ERROR;
                if (this->fn)
                    this->fn(TCP_ERROR, nullptr, 0);
            }

            if (!(this->isRecvBufferHasBeenDeleted))
            {
                if (buf)
                {
                    delete buf;
                }
            }
        }
        else
        {
            // disconnected
            if (this->autoReconnect && !this->isFromServer)
            {
                uint32_t t = millis();
                if (t - this->lastConnectTime > this->connectTimeout)
                {
                    ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "reconnecting...");
                    this->lastConnectTime = t;
                    this->handShake();
                }
            }
        }
    }

    bool CombinedServer::begin(uint16_t port)
    {
        this->server = new WiFiServer(port, 100);
        this->server->setNoDelay(true);
        this->server->begin();
        return true;
    }

    int CombinedServer::isWebSocketClientArrayHasFreeSapce()
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (nullptr == this->webSocketClients[i])
            {
                return i;
                break;
            }
        }
        return -1;
    }

    int CombinedServer::isHttpClientArrayHasFreeSpace()
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (nullptr == this->clients[i])
            {
                return i;
                break;
            }
        }
        return -1;
    }

    void CombinedServer::newWebSocketClientHandShanke(ExtendedWiFiClient *client, String request, int index)
    {
        // websocket
        int keyStart = request.indexOf("Sec-WebSocket-Key: "); // + 19;
        if (keyStart < 0)
        {
            // client->print("HTTP/1.1 403\r\n\r\n");
            // client->flush();
            client->stop();
            delete client;
            return;
        }

        // length of "Sec-WebSocket-Key: "
        keyStart += 19;

        // to generate server key
        String clientKey = request.substring(keyStart, request.indexOf("\r\n", keyStart));

        String serverKey = clientKey + String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        serverKey = generateServerKey(serverKey);

        if (!serverKey.length())
        {
            if (this->fn)
            {
                this->fn(nullptr, ERROR_GENERATE_KEY, nullptr, 0);
                client->stop();
                delete client;
                return;
            }
        }

        String response = "HTTP/1.1 101 Switching Protocols\r\n";
        response += "Connection: upgrade\r\n";
        response += "Upgrade: websocket\r\n";
        response += "Content-Length: 0\r\n";
        response += "Sec-WebSocket-Accept: " + serverKey + "\r\n\r\n";

        // give response to client
        client->print(response);
        client->flush();

        // transfer to a object of WebSocketClient
        WebSocketClient *webSocketClient = new WebSocketClient(client);

        // set callback
        webSocketClient->setCallBack(
            [this, webSocketClient](WebSocketEvents type, uint8_t *payload, uint64_t length)
            {
                // here don't process any events
                // all events will push to callback
                if (this->fn)
                    this->fn(webSocketClient, type, payload, length);
            });

        // push websocket client into queue
        this->webSocketClients[index] = webSocketClient;
    }

    int CombinedServer::findHttpCallback(String path)
    {
        if (this->nonWebSocketRequests.size() <= 0)
        {
            return -1;
        }

        for (int i = 0; i < this->nonWebSocketRequests.size(); i++)
        {
            if (path == this->nonWebSocketRequests.at(i)->path)
            {
                return i;
            }
        }
        return -1;
    }

    void CombinedServer::httpHandler(ExtendedWiFiClient *client, String *request)
    {
        // from http handshake
        if (request != nullptr)
        {
            if (request->length() > 0)
            {
                HttpMethod method;
                int pathStart = request->indexOf("GET ", 0); // + 4;

                if (pathStart < 0)
                {
                    pathStart = request->indexOf("POST ", 0); // + 5;
                    if (pathStart < 0)
                    {
                        ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "can not find path of http request");
                        return;
                    }
                    pathStart += 5;
                    method = POST;
                }
                else
                {
                    pathStart += 4;
                    method = GET;
                }

                int pathEnd = request->indexOf(" HTTP", pathStart);

                if (pathStart >= pathEnd)
                {
                    ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "can not find path of http request");
                    return;
                }

                // get path
                String path = request->substring(pathStart, pathEnd);

                if (path.length())
                {
                    // find GET process callback
                    int index = this->findHttpCallback(path);

                    if (index >= 0)
                    {
                        if (this->autoFillHttpResponseHeader)
                        {
                            // fill header
                            String res = "HTTP/1.1 " + String(this->nonWebSocketRequests.at(index)->code ? this->nonWebSocketRequests.at(index)->code : 200) + " OK\r\n";
                            res += "Content-Type: " + this->nonWebSocketRequests.at(index)->mimeType + "\r\n";
                            res += "Connection: keep-alive\r\n";
                            res += "Transfer-Encoding: chunked\r\n\r\n";
                            client->print(res);
                        }

                        if (this->nonWebSocketRequests.at(index)->fn)
                            this->nonWebSocketRequests.at(index)->fn(client, method, (uint8_t *)request, request->length());
                    }
                    else
                    {
                        ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "no http handler mathced: %s", path.c_str());
                        client->print("HTTP/1.1 404\r\n");
                        client->print("Connection: close\r\n");
                        client->print("Content-Type: text/plain\r\n");
                        client->print("Content-Length: 3\r\n\r\n404");
                        client->flush();
                        client->stop();
                    }
                }
                else
                {
                    client->stop();
                    ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "empty http request path");
                }
                return;
            }
            else
            {
                client->stop();
                ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "request is empty");
            }
        }

        // from loop
        if (this->publicPostHandler == nullptr)
        {
            return;
        }

        uint8_t *data = new (std::nothrow) uint8_t[MY_WEBSOCKET_HTTP_POST_LENGTH];

        if (!data)
        {
            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "memory full");

            if (this->fn)
                this->fn(nullptr, BUFFER_ALLOC_FAILED, nullptr, 0);
        }

        long len = client->read(data, MY_WEBSOCKET_HTTP_POST_LENGTH);

        if (len < 0)
        {
            // socket error
            client->stop();
            if (this->fn)
                this->fn(nullptr, TCP_ERROR, nullptr, 0);
        }
        else
        {
            // empty content
            return;
        }

        try
        {
            this->publicPostHandler->fn(client, NO_METHOD, data, len);
        }
        catch (std::exception &e)
        {
            delete data;
            data = nullptr;
        }

        if (data != nullptr)
        {
            delete data;
        }
    }

    void CombinedServer::loop()
    {
        if (this->server->hasClient())
        {
            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "A new client connected");
            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "Free Heap: %d", ESP.getFreeHeap());

            // get client
            WiFiClient newClient = this->server->available();

            // store fd
            ExtendedWiFiClient *client = new ExtendedWiFiClient(newClient);
            client->setNoDelay(true);

            // for process request data
            String request = "";

            if (client->available())
            {
                // read to uint8_t buffer is faster than arduino String
                bzero(this->headerBuffer, MY_WEBSOCKET_MAX_HEADER_LENGTH);
                int len = client->read(this->headerBuffer, MY_WEBSOCKET_MAX_HEADER_LENGTH);
                if (len < 0)
                {
                    client->stop();
                    delete client;

                    ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "error when read request header");
                    if (this->fn)
                        this->fn(nullptr, TCP_ERROR, nullptr, 0);

                    return;
                }

                if (len >= MY_WEBSOCKET_MAX_HEADER_LENGTH)
                {
                    client->stop();
                    delete client;

                    ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "request header length beyond max length");

                    if (this->fn)
                        this->fn(nullptr, MAX_HEADER_LENGTH_EXCEED, nullptr, 0);

                    return;
                }

                this->headerBuffer[len++] = 0;

                // convert to arduino String
                request = String((char *)this->headerBuffer);

                memset(this->headerBuffer + len, 0xff, MY_WEBSOCKET_MAX_HEADER_LENGTH - len);
            }
            else
            {
                return;
            }

            if (request.length() > 0)
            {
                if (request.indexOf("\r\n\r\n") >= 0)
                {
                    if (
                        request.indexOf("Connection: Upgrade") >= 0 &&
                        request.indexOf("Upgrade: websocket") >= 0 &&
                        request.indexOf("Sec-WebSocket-Key") >= 0)
                    {
                        // websocket request
                        int index = this->isWebSocketClientArrayHasFreeSapce();
                        if (index < 0)
                        {
                            // queue full
                            client->print("HTTP/1.1 404\r\nError: queue-full\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
                            client->flush();
                            client->stop();
                            delete client;
                            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "websocket queue full");
                        }
                        else
                        {
                            // process websocket request
                            this->newWebSocketClientHandShanke(client, request, index);
                        }
                    }
                    else
                    {
                        // http request
                        int index = this->isHttpClientArrayHasFreeSpace();
                        if (index >= 0)
                        {
                            this->clients[index] = client;
                            this->httpHandler(client, &request);
                        }
                        else
                        {
                            client->print("HTTP/1.1 404\r\nError: queue-full\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
                            client->flush();
                            client->stop();
                            delete client;
                            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "http queue full");
                        }
                    }
                }
            }
        }

        // loop websocket clients
        WebSocketClient *loopClient = nullptr;

        // loop http clients
        ExtendedWiFiClient *extendedclient = nullptr;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            // websocket
            loopClient = this->webSocketClients[i];
            if (nullptr != loopClient)
            {
                if (loopClient->connected())
                {
                    if (loopClient->available())
                    {
                        loopClient->loop();
                    }
                }
                else
                {
                    delete loopClient;
                    this->webSocketClients[i] = nullptr;
                    ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "A websocket client disconnected");
                }
            }

            // others
            extendedclient = this->clients[i];
            if (nullptr != extendedclient)
            {
                if (extendedclient->connected())
                {
                    if (extendedclient->available())
                    {
                        this->httpHandler(extendedclient, nullptr);
                    }
                }
                else
                {
                    delete extendedclient;
                    this->clients[i] = nullptr;
                    ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "A client disconnected");
                }
            }
        }
    }

    void CombinedServer::on(const char *path, NonWebScoketCallback fn, const char *mimeType, int statusCode, bool cover)
    {
        HttpCallback *cb = new HttpCallback();
        cb->path = String(path);
        cb->code = statusCode;
        cb->mimeType = String(mimeType);
        cb->fn = fn;

        if (cb->path.isEmpty() || !fn)
        {
            delete cb;
            ESP_LOGD(MY_WEBSOCKET_DEBUG_HEADER, "invalid path or callback for request");
            return;
        }

        bool stored = false;
        for (
            std::vector<HttpCallback *>::iterator it = this->nonWebSocketRequests.begin();
            it != this->nonWebSocketRequests.end();
            it++)
        {
            if ((*it)->path == cb->path)
            {
                if (cover)
                {
                    (*it)->fn = fn;
                    delete cb;
                    break;
                }
                else
                {
                    delete (*it);
                    this->nonWebSocketRequests.push_back(cb);
                    break;
                }
                stored = true;
            }
        }

        if (!stored)
        {
            this->nonWebSocketRequests.push_back(cb);
        }
    }

    CombinedServer::~CombinedServer()
    {
        // do some clean process

        // delete header buffer
        if (this->headerBuffer != nullptr)
        {
            delete this->headerBuffer;
        }

        // delete post handler
        if (this->publicPostHandler != nullptr)
        {
            delete this->publicPostHandler;
        }

        // delete http callbacks
        if (this->nonWebSocketRequests.size() > 0)
        {
            for (int i = 0; i < this->nonWebSocketRequests.size(); i++)
            {
                delete this->nonWebSocketRequests.at(i);
            }
        }

        // stop all clients and delete them
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (nullptr != this->clients[i])
            {
                this->clients[i]->stop();
                delete this->clients[i];
            }
            if (nullptr != this->webSocketClients[i])
            {
                this->webSocketClients[i]->stop();
                delete this->webSocketClients[i];
            }
        }

        // release vector memory
        std::vector<HttpCallback *>().swap(this->nonWebSocketRequests);
    }
};
