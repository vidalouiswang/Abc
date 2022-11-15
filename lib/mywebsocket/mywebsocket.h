/**
 * @file mywebsocket.h
 * @author Vida Wang (support@vida.wang)
 * @brief
 *
 * This is a small websocket server/client and http server component.
 * Fast speed, no memory leak.
 * Designed for ESP32 work with offical framework arduino-esp32 version 2.0.3.
 * Websocket client/server don't support wss.
 *
 * I'm not a native English speaker, so there may have many errors of words or grammar in comments, my apologies.
 *
 * 这是一个小型websocket客户端/服务器和http服务器组件。
 * 快速无内存泄漏。
 * 为ESP32设计，可与乐鑫官方arduino-esp32库2.0.3版本一同使用。
 * 不支持wss。
 *
 * @version 1.0.5
 * @date 2022-07-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MY_WEBSOCKET_H_
#define MY_WEBSOCKET_H_

#include <Arduino.h>
#include <WifiClient.h>
#include <vector>
#include <functional>
#include <mycrypto.h>
#include <WiFiServer.h>

#define myWebSocketHeader "GET @PATH@ HTTP/1.1\r\nHost: @HOST@\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nCache-Control: no-cache\r\nUpgrade: websocket\r\nSec-WebSocket-Key: @KEY@\r\n\r\n"

#define DEBUG_MYCRYPTO 1

#define READ_MAX_TIMES 100

// maximum payload length of websocket
// websocket消息最大长度
#define MY_WEBSOCKET_CLIENT_MAX_PAYLOAD_LENGTH 102400

// buffer length for websocket handshake
// 握手缓冲区长度
#define MY_WEBSOCKET_BUFFER_LENGTH 1024

// http post body length
// http post body长度
#define MY_WEBSOCKET_HTTP_POST_LENGTH 1024

// http header length
// http头最大长度
#define MY_WEBSOCKET_MAX_HEADER_LENGTH 1024

// client length
// 最大客户端数量
#define MAX_CLIENTS 10

#define MY_WEBSOCKET_DEBUG_HEADER "myWebsocket"

namespace myWebSocket
{
    /**
     * @brief generate keys for hankshake
     * 为握手生成密匙
     *
     * @param clientKey client key 客户端密匙
     * @return server key 服务器密匙
     */
    String generateServerKey(String clientKey);

    /**
     * @brief put all types together for one callback handle all events
     * 把所有的类型放在一起，这样可以用一个回调函数解决一切
     */
    typedef enum
    {
        TCP_CONNECTED = 0xfe,            // tcp connection established tcp连接已建立
        TCP_TIMEOUT = 0xfd,              // tcp connect timeout tcp连接超时
        TCP_FAILED = 0xfc,               // tcp connect failed tcp连接失败
        WS_CONNECTED = 0xfb,             // websocket connected websocket连接已建立
        WS_DISCONNECTED = 0xa8,          // websocket disconnected websocket连接已断开
        TCP_ERROR = 0xf8,                // tcp connection error tcp连接错误
        HANDSHAKE_UNKNOWN_ERROR = 0xf7,  // unknown error when handshake 握手时出现未知错误
        MAX_PAYLOAD_EXCEED = 0xf6,       // data length exceeded 数据超出最大长度
        BUFFER_ALLOC_FAILED = 0xf5,      // buffer allocate failed 缓冲区分配失败
        MAX_HEADER_LENGTH_EXCEED = 0xf4, // length of http header exceeded http头长度超出最大长度
        REACH_MAX_READ_TIMES = 0xae,     // reach maximum retry count 超过最大重试次数
        ERROR_GENERATE_KEY = 0xac,       // error when generate key 生成密匙时发生错误

        // the following types for websocket data frame
        // 下面的类型是websocket数据帧类型

        TYPE_CON = 0x00,    // extra data frame 附加数据帧
        TYPE_TEXT = 0x01,   // text frame 文本数据帧
        TYPE_BIN = 0x02,    // binary frame 二进制数据帧
        TYPE_CLOSE = 0x08,  // close frame 关闭连接数据帧
        TYPE_PING = 0x09,   // ping frame ping数据帧
        TYPE_PONG = 0x0a,   // pong frame pong数据帧
        TYPE_UNKNOWN = 0xff // reserved 保留位
    } WebSocketEvents;

    /**
     * @brief add some functions to WiFiClient
     * 给WiFiClient添加几个函数
     */
    class ExtendedWiFiClient : public WiFiClient
    {
    public:
        inline ExtendedWiFiClient() {}
        inline ExtendedWiFiClient(const WiFiClient &externalClient) : WiFiClient(externalClient) {}
        inline ~ExtendedWiFiClient() {}

        //

        /**
         * @brief this is for default method with "Transfer-Encoding: chunked"
         * 默认的分段发送数据函数
         *
         * @param content data to send, pointer of class String object
         * 需要发送的数据，一个String类的对象的指针
         * @return bytes had been sent 已发送的数据的字节数
         */
        inline uint64_t send(String *content)
        {
            if (!content)
                return 0;
            if (!content->length())
                return 0;
            String *res = new String(content->c_str());
            *res = String(res->length(), HEX) + "\r\n" + *res + "\r\n";
            size_t len = 0;
            try
            {
                len = this->print(*res);
            }
            catch (std::exception &e)
            {
            }

            delete res;
            return len;
        }

        /**
         * @brief this is for default method with "Transfer-Encoding: chunked"
         * 默认的分段发送数据函数
         *
         * @param content data to send, c string, c字符串
         * 需要发送的数据，c字符串
         * @return bytes had been sent 已发送的数据的字节数
         */
        inline uint64_t send(const char *content)
        {
            if (!content)
                return 0;
            String *res = new String(content);
            auto len = this->send(res);
            delete res;
            return len;
        }

        /**
         * @brief send zero block and close socket
         * 发送结束数据块然后断开socket
         */
        inline void close()
        {
            this->print("0\r\n\r\n");
            this->flush();
            this->stop();
        }
    };

    /**
     * @brief websocket client message callback
     * websocket 客户端消息回调函数
     */
    typedef std::function<void(WebSocketEvents type, uint8_t *payload, uint64_t length)> WebSocketMessageCallback;

    class WebSocketClient
    {
    private:
        /**
         * @brief websocket server host name
         * websocket服务器主机
         */
        String host = "";

        /**
         * @brief id set by user for other purpose
         * 用户设置的用于其他用途的id
         */
        int id = -1;

        /**
         * @brief websocket server port
         * websocket服务器端口
         */
        uint16_t port = 80;

        /**
         * @brief websocket request path
         * websocket请求路径
         */
        String path = "/";

        /**
         * @brief for handshake websocket握手
         *
         * @return true 握手成功
         * @return false 握手失败
         */
        bool handShake();

        /**
         * @brief host, port, and path for handshake
         * 主机名、端口、路径用于握手
         */
        String domain = "";

        /**
         * @brief websocket message callback
         * websocket消息回调函数
         */
        WebSocketMessageCallback fn = nullptr;

        /**
         * @brief for basic tcp connection
         * 建立底层tcp连接
         */
        ExtendedWiFiClient *client = new ExtendedWiFiClient();

        /**
         * @brief consecutive frame buffer
         * 连续帧缓冲区
         */
        uint8_t *accBuffer = nullptr;

        /**
         * @brief offset of consecutive frame buffer
         * 连续帧缓冲区偏移量
         */
        uint64_t accBufferOffset = 0;

        /**
         * @brief reconnect after connection lost or not
         * 是否在断开连接后自动重连
         */
        uint8_t autoReconnect = 0xffu;

        /**
         * @brief recv buffer has been deleted by user or not
         * 接收缓冲区是否被用户主动释放
         */
        bool isRecvBufferHasBeenDeleted = false;

        /**
         * @brief to record last connect time
         * 记录上次尝试连接的时间
         */
        uint32_t lastConnectTime = 0;

        /**
         * @brief a timeout when websocket disconnected from server and reconnect
         * 当websocket与服务器丢失连接之后多久再次重新尝试连接
         */
        uint32_t connectTimeout = 5000;

        /**
         * @brief buffer for handshake
         * websocket 握手缓冲区
         */
        uint8_t *buffer = new uint8_t[MY_WEBSOCKET_BUFFER_LENGTH];

        /**
         * @brief a key generated as client key
         * 客户端密匙
         */
        String clientKey;

        /**
         * @brief to generate information for handshake
         * 为websocket握手生成需要的数据
         *
         * @return websocket header for handshake 用于websocket握手的头数据
         */
        String generateHanshake();

        /**
         * @brief internal universal send function, send data to server or client, attention attached
         * 通用的内部发送数据函数，用于把数据发送到客户端或者服务器，请阅读注意事项
         *
         * @param type type of websocket frame, websocket数据帧类型
         * @param data payload 数据
         * @param len length of payload 数据长度
         * @return how many data have been sent in bytes 发送了多少字节数据
         *
         * @attention the data stored in pointer "data" will be changed because of the masking process
         * (when act as a client), you should make a copy before send if you didn't want this function
         * mofidy original data
         *
         * 客户端在发送数据时，存储于指针"data"中的数据会被masking过程修改，如果你还需要使用这些原始数据，你可以在发送数据
         * 之前对原始数据创建一份拷贝
         */
        uint64_t _send(WebSocketEvents type, uint8_t *data, uint64_t len);

    public:
        /**
         * @brief to indicate current status of client
         * 用于指示当前client的状态
         */
        WebSocketEvents status;

        /**
         * @brief default constructor
         * 默认构造函数
         */
        inline WebSocketClient() {}

        // this will be true if this client is transfer from local websocket server
        // the client will not reconnect automaticlly if this is true

        /**
         * @brief to indicate current object is transfer from local server or not, attention attached
         * 用于指示当前对象是否是由本地websocket服务器转换而来，请阅读注意事项
         *
         * @attention the client won't reconnect automatically if this varible set to true
         * 客户端不会自动重新连接如果这个变量被设置为true
         */
        uint8_t isFromServer = 0;

        // only CombinedServer will call this function
        // to transfer the client in

        /**
         * @brief a special constructor only for CombinedServer, to transfer client in
         * 一个特殊的构造函数仅被CombinedServer类的对象使用，用于传输socket连接
         *
         * @param client an object of ExtendedWiFiClient(socket) 一个ExtendedWiFiClient的对象(套接字)
         */
        inline WebSocketClient(ExtendedWiFiClient *client)
        {
            if (this->client)
            {
                try
                {
                    this->client->stop();
                    delete this->client;
                }
                catch (std::exception &e)
                {
                }
            }
            this->client = client;
            this->isFromServer = 0xffu;
            this->status = WS_CONNECTED;
            this->autoReconnect = 0;
        }

        /**
         * @brief Destroy the Web Socket Client object
         * 析构函数
         */
        inline ~WebSocketClient()
        {
            // do clean process
            // 执行清理
            delete this->buffer;
            if (this->client != nullptr)
            {
                delete this->client;
            }
        }

        /**
         * @brief to check socket if available
         * 查看套接字是否连接正常
         *
         * @return true connected 正常
         * @return false disconnected or others 断开或其他
         */
        inline bool available()
        {
            return this->client->available();
        }

        /**
         * @brief to check socket if available(only a name different from available)
         * 查看套接字是否连接正常(只是名字和available不一样)
         *
         * @return true connected 正常
         * @return false disconnected or others 断开或其他
         */
        inline uint8_t connected()
        {
            return this->client->connected();
        }

        /**
         * @brief connect to remote server, error code will transfered in if callback has been set
         * or if error occurred when connecting, you could check status manually
         * 连接到远程服务器，如果设置了回调函数，如果在连接中发生了错误，回调函数会被执行，错误代码会被传送到回调函数的
         * "type"
         *
         * @param host host name 主机名
         * @param port port of server 主机端口号
         * @param path path of websocket, websocket路径
         * @return true connected 已连接
         * @return false error when connecting 连接时发生错误
         */
        inline bool connect(String host, uint16_t port, String path = "/")
        {
            this->host = host;
            this->port = port;
            this->path = path;
            this->status = WS_DISCONNECTED;
            return this->handShake();
        }

        /**
         * @brief connect to remote server, error code will transfered in if callback has been set
         * or if error occurred when connecting, you could check status manually
         * 连接到远程服务器，如果设置了回调函数，如果在连接中发生了错误，回调函数会被执行，错误代码会被传送到回调函数的
         * "type"
         *
         * @param url full address, examples: 完整的地址，例如:
         * ws://abc.com or
         * ws://abc.com:8080 or
         * ws://abc.com:8080/abc or
         * abc.com or
         * abc.com:8080
         * ...
         *
         * @return true connected 已连接
         * @return false error when connecting 连接时发生错误
         */
        bool connect(String url);

        /**
         * @brief connect to remote server, error code will transfered in if callback has been set
         * or if error occurred when connecting, you could check status manually
         * 连接到远程服务器，如果设置了回调函数，如果在连接中发生了错误，回调函数会被执行，错误代码会被传送到回调函数的
         * "type"
         *
         * @param url full address, examples: 完整的地址，例如:
         * ws://abc.com or
         * ws://abc.com:8080 or
         * ws://abc.com:8080/abc or
         * abc.com or
         * abc.com:8080
         * ...
         *
         * @return true connected 已连接
         * @return false error when connecting 连接时发生错误
         */
        inline bool connect(const char *url)
        {
            return this->connect(String(url));
        }

        /**
         * @brief connect to remote server, error code will transfered in if callback has been set
         * or if error occurred when connecting, you could check status manually
         * 连接到远程服务器，如果设置了回调函数，如果在连接中发生了错误，回调函数会被执行，错误代码会被传送到回调函数的
         * "type"
         *
         * @param host host name 主机名
         * @param port port of server 主机端口号
         * @param path path of websocket, websocket路径
         * @return true connected 已连接
         * @return false error when connecting 连接时发生错误
         */
        inline bool connect(const char *host, uint16_t port, const char *path)
        {
            return this->connect(String(host), port, String(path));
        }

        /**
         * @brief set websocket event callback, include all types of events
         * buffer will be set to nullptr and length will be set to 0 if callback
         * called because of not a websocket data frame event
         *
         * 设置websocket的回调函数，包含所有事件的类型，如果回调函数不是因为数据被执行，
         * 则buffer会被设置成空指针，长度会被设置成0
         *
         * @param fn callback 回调函数
         */
        inline void setCallBack(WebSocketMessageCallback fn)
        {
            this->fn = fn;
        }

        /**
         * @brief main loop of websocket client, websocket客户端的主循环
         *
         */
        void loop();

        /**
         * @brief mark the buffer that use for contain data from socket has been removed
         * by user, attention attached
         *
         * 标记用于存放从socket接收到数据的缓冲区已经被用于释放，请阅读注意事项
         *
         * @attention be careful use this function, if you forget delete buffer but this function
         * had been called, you will got memory leak
         *
         * 小心使用这个函数，因为如果你忘记了释放缓冲区而又调用了这个函数，会发生内存泄漏
         */
        inline void setRecvBufferDeleted() { this->isRecvBufferHasBeenDeleted = true; }

        /**
         * @brief set a special id by user for other purpose
         * 由用户设置一个特殊的ID用于其他用途
         *
         * @param id
         */
        inline void setID(int id) { this->id = id; }

        /**
         * @brief get id set by user 获取由用户自行设置的id
         *
         * @return id set by user 用户设置的id
         */
        inline int getID() { return this->id; }

        /**
         * @brief send a string 发送一个字符串
         *
         * @param data a pointer to String object 一个指向String类对象的指针
         * @return bytes had been sent 已被发送的数据字节数
         */
        inline uint64_t send(String *data)
        {
            return this->send(data->c_str());
        }

        /**
         * @brief send a string 发送一个字符串
         *
         * @param data an object of class String  一个指向String类的对象
         * @return bytes had been sent 已被发送的数据字节数
         */
        inline uint64_t send(String data)
        {
            return this->send(data.c_str());
        }

        /**
         * @brief send a string 发送一个字符串
         *
         * @param data c string, c字符串
         * @return bytes had been sent 已被发送的数据字节数
         */
        uint64_t send(const char *data);

        /**
         * @brief send binary data 发送二进制数据
         *
         * @param data pointer of binary array 二进制数组的指针
         * @param length length of binary array 数组长度
         * @return bytes had been sent 已被发送的数据字节数
         */
        inline uint64_t send(uint8_t *data, uint64_t length)
        {
            if (!data || !length)
                return 0;
            return this->_send(TYPE_BIN, data, length);
        }

        /**
         * @brief send c string as binary type 以二进制数据格式发送c字符串
         *
         * @param data pointer of c string, c字符串
         * @return bytes had been sent 已被发送的数据字节数
         */
        inline uint64_t send(char *data)
        {
            if (!data)
                return 0;

            uint32_t length = strlen(data);

            if (!length)
                return 0;
            return this->_send(TYPE_BIN, (uint8_t *)data, length);
        }

        /**
         * @brief manual hard disconnect from websocket, attention attached
         * 手动强制断开websocket连接，请阅读注意事项
         *
         * @attention client will NOT reconnect after this function called
         * 当此函数被执行后，客户端 [不再] 自动连接
         *
         */
        inline void stop()
        {
            this->autoReconnect = 0;
            this->client->stop();
        }

        /**
         * @brief alias of "stop". manual hard disconnect from websocket, attention attached
         * "stop"的别名。手动强制断开websocket连接，请阅读注意事项
         *
         * @attention client will NOT reconnect after this function called
         * 当此函数被执行后，客户端 [不再] 自动连接
         *
         */
        inline void disconnect()
        {
            this->stop();
        }

        /**
         * @brief set client reconnect after connection lost
         * 设置客户端是否自动重连
         *
         * @param autoReconnect true: auto reconnect 自动重连, false: will NOT auto reconnect [不]自动重连
         * @param timeout timeout for auto reconnect after connection lost, in milliseconds
         * 断开后多久自动重连的超时时间，以毫秒计
         */
        inline void setAutoReconnect(bool autoReconnect = true, uint64_t timeout = 5000)
        {
            this->autoReconnect = autoReconnect ? 0xffu : 0;
            this->connectTimeout = timeout;
        }
    };

    /**
     * @brief currently support get and post
     * 目前支持get和post
     */
    typedef enum
    {
        GET,
        POST,
        NO_METHOD,
        OTHERS
    } HttpMethod;

    // for universal websocket server message callback
    // 通用的websocket服务器消息回调函数
    typedef std::function<void(WebSocketClient *client, WebSocketEvents type, uint8_t *payload, uint64_t length)> WebSocketServerCallback;

    // for http handler
    // http请求回调函数
    typedef std::function<void(ExtendedWiFiClient *client, HttpMethod method, uint8_t *data, uint64_t length)> NonWebScoketCallback;

    /**
     * @brief http callback arguments
     * http回调函数参数
     */
    typedef struct
    {
        String path;
        int code = 0;
        String mimeType;
        NonWebScoketCallback fn = nullptr;
    } HttpCallback;

    /**
     * @brief this class handles http and websocket requests
     * 这个类处理http和websocket请求
     */
    class CombinedServer
    {
    private:
        /**
         * @brief http clients
         * http客户端
         */
        ExtendedWiFiClient *clients[MAX_CLIENTS] = {nullptr};

        /**
         * @brief websocket clients
         * websocket客户端
         */
        WebSocketClient *webSocketClients[MAX_CLIENTS] = {nullptr};

        /**
         * @brief server request callback
         * 服务器请求回调函数
         */
        WebSocketServerCallback fn = nullptr;

        /**
         * @brief router table
         * 路由表
         */
        std::vector<HttpCallback *> nonWebSocketRequests;

        /**
         * @brief base server object
         * 底层服务器
         */
        WiFiServer *server = nullptr;

        /**
         * @brief public post handler
         * 公用的post请求处理器
         */
        HttpCallback *publicPostHandler = nullptr;

        /**
         * @brief mark if should fill http header or not
         * if this set to true you should only provide main content of html/js/css...
         * otherwise you could process raw data in callback edit your own response header
         *
         * 如果这个变量设置为true，那么你只需要提供响应的主要内容
         * 否则你需要自己构造http头
         */
        bool autoFillHttpResponseHeader = true;

        // buffer for http request header
        // http请求头缓冲区
        uint8_t *headerBuffer = new uint8_t[MY_WEBSOCKET_MAX_HEADER_LENGTH];

        /**
         * @brief check if there have free space in websocket clients queue
         * 查找websocket客户端队列是否有空余位置
         *
         * @return index of free space 空余的位置, -1 == full 返回 -1 则队列已满
         */
        int isWebSocketClientArrayHasFreeSapce();

        /**
         * @brief check if there have free space in http clients queue
         * 查找http客户端队列是否有空余位置
         *
         * @return index of free space 空余的位置, -1 == full 返回 -1 则队列已满
         */
        int isHttpClientArrayHasFreeSpace();

        /**
         * @brief find http callback for http request path
         * 查找对应请求路径的http处理函数
         *
         * @param path path of http request http请求的路径
         * @return index of callback 处理函数的position, -1 == nothing 返回 -1 代表没有对应路径的处理函数
         */
        int findHttpCallback(String path);

        /**
         * @brief http handler http请求处理函数
         *
         * @param client socket 套接字
         * @param request http request header, http请求头
         */
        void httpHandler(ExtendedWiFiClient *client, String *request);

        /**
         * @brief a client require upgrade to websocket protocols
         * 客户端请求升级到websocket连接
         *
         * @param client socket 套接字
         * @param request request header 请求头
         * @param index position of queue in websocket queue, websocket队列空余的位置
         */
        void newWebSocketClientHandShanke(ExtendedWiFiClient *client, String request, int index);

    public:
        inline CombinedServer() {}
        ~CombinedServer();

        /**
         * @brief set universal server callback
         * 设置通用服务器回调函数
         *
         * @param fn callback 回调函数
         */
        inline void setCallback(WebSocketServerCallback fn)
        {
            this->fn = fn;
        }

        //
        //

        /**
         * @brief set whether to automatically fill header
         * if you'd like process raw data then you should call
         * this function at first and provide false in argument
         *
         * 设置是否自动填充http头
         * 如果你想自己处理http头你应该先调用此函数且提供false为参数
         *
         * @param autoFill true: fill http header automatically
         * 自动填充http头,
         * false: will not fill http header
         * 不会自动填充http头
         */
        inline void setAutoFillHttpResponseHeader(bool autoFill)
        {
            this->autoFillHttpResponseHeader = autoFill;
        }

        /**
         * @brief set process handler for http request
         * 设置http请求处理函数
         *
         * @param path path of http request, http请求路径
         * @param fn callback 回调函数
         * @param mimeType mime type 源类型
         * @param statusCode http status code, http响应状态码
         * @param cover cover former callback or not 是否覆盖之前的回调函数
         */
        void on(const char *path,
                NonWebScoketCallback fn,
                const char *mimeType = "text/html;charset=utf-8",
                int statusCode = 200,
                bool cover = true);

        /**
         * @brief set process handler for http request
         * 设置http请求处理函数
         *
         * @param path path of http request, http请求路径
         * @param fn callback 回调函数
         * @param mimeType mime type 源类型
         * @param statusCode http status code, http响应状态码
         * @param cover cover former callback or not 是否覆盖之前的回调函数
         */
        inline void on(String path,
                       NonWebScoketCallback fn,
                       String mimeType = "text/html;charset=utf-8",
                       int statusCode = 200,
                       bool cover = true)
        {
            this->on(path.c_str(), fn, mimeType.c_str(), cover);
        }

        /**
         * @brief set public http post request handler
         * 设置公用的http post请求处理回调函数，这个功能我没用过
         *
         * @param fn callback 回调函数
         */
        inline void setPublicPostHandler(NonWebScoketCallback fn)
        {
            if (!fn)
                return;

            if (this->publicPostHandler != nullptr)
            {
                delete this->publicPostHandler;
            }
            this->publicPostHandler = new HttpCallback();
            this->publicPostHandler->path = "";
            this->publicPostHandler->fn = fn;
        }

        /**
         * @brief start server 开启服务器
         *
         * @param port port of server 服务器端口
         * @return true
         */
        bool begin(uint16_t port = 80);

        /**
         * @brief main loop of server 服务器主循环
         *
         */
        void loop();
    };
} // namespace myWebSocket

#endif