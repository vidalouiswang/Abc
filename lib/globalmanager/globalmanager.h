/**
 * @file globalmanager.h
 * @author Vida Wang (support@vida.wang)
 * @brief This file is manager of whole system.
 * 这个文件是整个系统的管理器。
 *
 * @version 1.0.0
 * @date 2022-08-19
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef GLOBAL_MANAGER_H_
#define GLOBAL_MANAGER_H_

#include <Arduino.h>
#include <vector>
#include <map>
#include <WiFi.h>
#include <functional>
#include <config.h>
#include <mydb.h>
#include <mywebsocket.h>
#include "../../src/app/app.h"
#include <ota.h>
#include <provider.h>
#include <softtimer.h>

// #include <DNSServer.h>

// disposable hash
// 一次性密匙
typedef struct
{
    /**
     * @brief unix epoch timestamp
     * unix时间戳
     *
     */
    uint64_t time = 0;

    /**
     * @brief SHA256 digest
     * SHA256数字摘要
     *
     */
    uint8_t *hash = nullptr;
} OneTimeAuthorization;

typedef enum : uint32_t
{
    GT_START_AP = 0b1,
    GT_DETECT_REMOTE_SERVER = 0b10,
    GT_CLOSE_AP = 0b100
} GlobalTask;

// firmware compile time
// 固件编译时间
#define FIRMWARE_COMPILE_TIME "@2024/05/01-20:33:28"

void otaTask(void *);
//void APScheduler(void *t);

typedef std::function<void(myWebSocket::WebSocketClient *client,
                           myWebSocket::WebSocketEvents event,
                           uint8_t *data,
                           uint64_t length,
                           std::vector<Element *> *output)>
    WebSocketCallback;

class GlobalManager
{
private:
    /**
     * @brief esp32 id for communication, uint8 array
     * esp32 id, 用于通信，uint8数组
     */
    Element UniversalID;

    /**
     * @brief administrator user name, uint8 array
     * 管理员用户名，uint8 数组
     */
    Element userName;

    /**
     * @brief administrator password, uint8 array
     * 管理员密码, uint8数组
     */
    Element password;

    /**
     * @brief nickname of current device, string
     * 当前设备昵称，字符串
     */
    Element nickname;

    /**
     * @brief indicate that boot with new firmwate throught ota update
     * 指示当前启动固件是否是刚才通过ota升级的
     */
    bool isNewFirmwareBoot = false;

#ifdef REBOOT_IF_PROACTIVE_DETECT_REMOTE_SERVER_OFFLINE
    /**
     * @brief number of proactive detected server offline
     * 检测到服务器下线的次数
     */
    uint16_t remoteServerOfflineDetectedTimes = 0;
#endif

    /**
     * @brief user custom websocket client callback
     * this function will called when incomming command
     * didn't match any built-in commands
     *
     * 用户自定义的websocket客户端回调函数
     * 当所有内建命令均未命中时
     * 此回调函数会被执行
     */
    WebSocketCallback extraWifiWebsocketCallback = nullptr;

    /**
     * @brief user custom websocket server callback
     * this function will be called when incomming command
     * didn't match any built-in commands
     *
     * 用户自定义的websocket服务器回调函数
     * 当所有内建命令均未命中时
     * 此回调函数会被调用
     */
    WebSocketCallback extraAPWebsocketCallback = nullptr;

    /**
     * @brief mark ssid and password for remote connection existed in database
     * 标识数据库中是否存在用于远程连接的wifi ssid 和密码
     */
    uint8_t isWiFiInfoOK = 0;

    /**
     * @brief indicate that wifi connected
     * 指示wifi是否已连接
     */
    uint8_t isWifiConnected = 0;

    /**
     * @brief to stored used hashes for authorization
     * 用于存储已使用的一次性密匙
     *
     * @note because of the time between control device and esp32 may have gap
     * so used hashes will be stored in a map container
     * every pair will consume about 130 bytes ram
     * those pair will be deleted automatically
     * refer the settings in "config.h"
     *
     * 因为控制设备和esp32之间存在时间差，这个时间差可能很大
     * 所以无法用时间戳来判定是否通过认证，所以用个map把用过的
     * 一次性密匙存起来，每一条数据大约占用130字节内存
     * 这些存储的内容会被自动删除
     * 相关设置在 "config.h" 里
     */
    std::map<uint64_t, Element *> usedHashes;

    // dns server for local
    // DNSServer *dnsServer;

    /**
     * @brief local server for AP mode to handle http and websocket requests
     * 本地服务器用来处理AP模式的http和websocket连接
     */
    myWebSocket::CombinedServer *apServer = new myWebSocket::CombinedServer();

    /**
     * @brief websocket client to commuicate with remote server
     * 用于与远程服务器通信的websocket客户端
     */
    myWebSocket::WebSocketClient *websocketClient = new myWebSocket::WebSocketClient();

    /**
     * @brief the time that last time detect remote server is online proactively
     * 上次主动探测远程服务器是否在线的时间
     */
    uint64_t lastDetectServerOnlineTime = 0;

    /**
     * @brief indicate use wifi or not
     * 指示是否使用wifi
     */
    uint8_t isWifiEnabled = 0xffu;

    /**
     * @brief this pin will be used as when to enable wifi
     * for example, your application use BT or BLE
     * but you'd like to enable wifi to use OTA update
     * at a moment, at this time, you could connect this
     * pin to 3.3V or GND, it will enable wifi at this boot
     *
     * 这个引脚被设置用来启动wifi
     * 举例，你的应用平时都只使用BT或BLE，
     * 为了避免共存问题，平时wifi都是关闭的
     * 但是你可能有时候想要使用OTA升级固件，
     * 这个时候你可以将这个引脚连接到3.3V或数字地
     * 这次启动就会启动wifi
     *
     */
    uint8_t enableWifiPin = 0;

    uint32_t tDetectServer = 0;

    /**
     * @brief indicate remote server is online
     * 指示远程服务器是否在线
     */
    uint8_t isServerOnline = 0;

    /**
     * @brief domain for websocket client to remote server
     * 用于连接远程websocket服务器的域名
     */
    String globalRemoteWebsocketDomain = "";

    /**
     * @brief port for websocket client to remote server
     * 用于连接远程websocket服务器的端口
     */
    uint16_t globalRemoteWebsocketPort = 0;

    /**
     * @brief path for websocket client to remote server
     * 用于连接远程websocket服务器的路径
     */
    String globalRemoteWebsocketPath = "";

    /**
     * @brief connect to an access point provided by a wireless router
     * 连接到热点(俗称路由器)
     *
     */
    void connectWifi();

    /**
     * @brief universal callback for websocket client and server
     * 全局通用的用于websocket客户端和服务器的回调函数
     *
     * @param client instance of websocket client, websocket客户端实例
     * @param event event 事件
     * @param data payload 数据负载
     * @param length length of payload 数据负载的长度
     */
    void internalUniversalWebsocketCallback(myWebSocket::WebSocketClient *client,
                                            myWebSocket::WebSocketEvents event,
                                            uint8_t *data,
                                            uint64_t length);

    /**
     * @brief handle incomming data sent from remote server
     * 处理从远程服务器发来的数据
     *
     * @param event event 事件
     * @param data payload( this pointer transfered because of
     * it may has other purpoes for use) 数据负载(之所以传送这个指针
     * 是因为可能有其他的作用)
     * @param length length of payload 数据负载的长度
     * @param output decoded vector of payload, contains pointer of Element object;
     * 已解码的装有Element类对象指针的vector
     */
    void internalRemoteMsgHandler(
        myWebSocket::WebSocketEvents event,
        uint8_t *data,
        uint64_t length,
        std::vector<Element *> *output);

    /**
     * @brief handle incomming data sent from websocket client for AP mode
     * 处理AP模式下websocket客户端发来的数据
     *
     * @param client instance of websocket client 客户端实例
     * @param event event 事件
     * @param data payload( this pointer transfered because of
     * it may has other purpoes for use) 数据负载(之所以传送这个指针
     * 是因为可能有其他的作用)
     * @param length length of payload 数据负载的长度
     * @param output decoded vector of payload, contains pointer of Element object;
     * 已解码的装有Element类对象指针的vector
     */
    void internalLocalMsgHandler(
        myWebSocket::WebSocketClient *client,
        myWebSocket::WebSocketEvents event,
        uint8_t *data,
        uint64_t length,
        std::vector<Element *> *output);

    /**
     * @brief connect to remote server
     * 连接到远程websocket服务器
     */
    void connectWebocket();

    /**
     * @brief the time that last try to connect to access point
     * 上次尝试连接到热点的时间
     */
    uint64_t lastConnectWiFiTime = 0;

    /**
     * @brief load config for remote websocket connection
     * 加载用于连接远程websocket服务器的信息
     *
     */
    void loadWebsocketInformation();

    /**
     * @brief ensure that new firmware through ota update
     * is valid, otherwise it will rollback automatically
     * config refer to config.h
     *
     * 确保通过ota升级的新固件没有错误，否则将自动回滚固件
     * 设置在 config.h 里
     */
    void makeSureNewFirmwareValid();

    /**
     * @brief generate a buffer contains user name of other users
     * this buffer will be send to server when registering
     *
     * 生成一个包含其他用户 用户名的缓存
     * 当向服务器注册时会发送此缓存
     */
    void buildUsersBuffer();

    /**
     * @brief generate a buffer contains information
     * of all providers
     * this buffer will be send to control device when
     * user login
     *
     * 生成一个包含所有provider信息的缓存，用户登录时会接收到此缓存
     * 用于显示
     *
     * @param buildAll if false provided, this function
     * only build normal providers, not include built-in providers
     *
     * 如果此参数提供 false 则仅构建不包括内建provider的那些provider
     */
    void buildProvidersBuffer(bool buildAll = true);

    /**
     * @brief indicate provider buffer shrank or not
     *
     * 指示 provider buffer 是否被缩减过
     *
     */
    bool isProviderBufferShrank = false;

    /**
     * @brief register buffer
     * 注册用的缓存
     */
    uint8_t *registerBuffer = nullptr;

    /**
     * @brief length of register buffer
     * 注册用缓存的长度
     */
    uint32_t lengthOfRegisterBuffer = 0;

    /**
     * @brief generate register buffer
     * this buffer will be send to server
     * when websocket client connected
     *
     * 生成一个注册缓存，当websocket客户端
     * 建立连接后会发送此缓存到服务器
     */
    void buildRegisterBuffer();

    /**
     * @brief user or another device require to
     * execute callback of a provider
     *
     * 用户或其他设备要求执行一个provider的回调函数
     *
     * @param output decoded data, a pointer of vector
     * contains pointers of Element object
     *
     * 已解码的数据，一个vector容器，存储了Element类对象的指针
     *
     * @param response response 响应
     */
    void executeCommand(Elements *output, Elements *response);

    /**
     * @brief fill response with basic information of current device
     * 填充当前设备基本信息到响应
     *
     * @param userID id of user 用户ID
     * @param response response 响应
     * @param isAdmin has administrator privileges or not 是否具有管理员权限
     */
    void getFindDeviceBuffer(const char *userID, Elements *response, bool isAdmin);

    /**
     * @brief cotainer of providers
     * provider的容器
     */
    std::vector<Provider *> *providers = new std::vector<Provider *>();

    /**
     * @brief send command to another device
     * 发送命令给另一个设备
     *
     * @attention this function still developing
     * 这个功能还在开发中
     *
     * @tparam T uint8 | uint64
     * @param frinedID universal id of target device
     * @param command command 命令
     * @param providerID id of provider on target device 对方设备上的provider id
     * @param request optional 可选项
     * @return true success 成功
     * @return false failed 失败
     */
    template <class T>
    bool _sendBundle(String frinedID, T command, uint16_t providerID, Elements *request = nullptr);

    Element eLogCommand;
    Element eAdminIDStr;

public:
    /**
     * @brief send hello request or response to server
     * 发送打招呼请求或者回应服务器
     *
     * @param hello send hello or world 发送hello还是world
     * @param client which client should use 使用哪个客户端
     */
    void sendHello(bool hello = true, myWebSocket::WebSocketClient *client = nullptr);

    /**
     * @brief remote websocket client connected time, system running time
     * 远程websocket客户端建立连接的时间，系统运行时间
     */
    unsigned long remoteWebsocketConnectedTimestamp = 0;

    /**
     * @brief this object handle ota update process
     * 这个对象用来处理ota升级过程
     */
    WebsocketOTA *ota = nullptr;

    /**
     * @brief the time that ota update process started
     * ota升级开始的时间
     */
    uint64_t otaStartTime = 0;

    /**
     * @brief DO NOT change contents following 3 lines, tools will generate it automatically
     * 不要修改下面三行内容，这是工具自动生成替换进去的
     */
    //apindex Sun Apr 02 2023 19:36:05 GMT+0800 (China Standard Time)
const char* serverIndex = R"(<!DOCTYPE html><html><head> <title>Abc</title> <meta name='viewport' content='width=device-width,minimum-scale=1.0,maximum-scale=1.0,user-scalable=no' /> <meta charset='utf-8' /> <style> .divMain { position: fixed; left: 50%; top: 50%; transform: translate(-50%, -50%); padding: 0.3rem; text-align: center; } input { text-align: center; font-size: 1.1rem; margin: 0.3rem; } .btn { font-size: 1.1rem; text-decoration: none; color: #000; border: 0.1rem solid #000; border-radius: 0.1rem; margin: 0.3rem; padding: 0.2rem; display: inline-block; } .btn:hover { background: lightskyblue; border-radius: 0; } .msg { position: fixed; width: 100vw; left: 0; padding: 0.3rem; background: rgb(0, 91, 134); color: #fff; text-align: center; top: -100%; font-size: 1.1rem; transition: all 0.9s ease-in-out; } .msgShow { top: 0; } </style></head><body> <div id='msg' class='msg'></div> <div class='divMain'> <div> <div> <input id='ssid' type='text' /> </div> <div> <input id='wifiPwd' type='password' /> </div> </div> <div> <div> <input id='user' type='text' /> </div> <div> <input id='userPwd' type='text' /> </div> </div> <div> <div> <input id='websocketDomain' type='text' /> </div> <div> <input id='websocketPort' type='text' /> </div> <div> <input id='websocketPath' type='text' /> </div> </div> <div> <div> <input id='nickname' type='text' /> </div> <div> <input id='token' type='password' /> </div> </div> <div> <div> <a class='btn' id='setArguments'></a> </div> <div> <a class='btn' id='reboot'></a> </div> <div> <a class='btn' id='rebootNow'></a> </div> <div> <a class='btn' id='rollback'></a> </div> <div> <a class='btn' id='deepSleep'></a> </div> </div> </div> <script> (function(){function e(e){for(let t=0;t<e.byteLength;++t)e.arr.push(0);let t=new Uint8Array(e.arr),n=new DataView(t.buffer);switch(e.type){case'8':n.setUint8(t.byteLength-e.byteLength,e.number);break;case'16':n.setUint16(t.byteLength-e.byteLength,e.number,!0);break;case'32':n.setUint32(t.byteLength-e.byteLength,e.number,!0);break;case'64':n.setBigUint64(t.byteLength-e.byteLength,BigInt(e.number),!0);break;case'f':n.setFloat32(t.byteLength-e.byteLength,e.number,!0);break;case'd':n.setFloat64(t.byteLength-e.byteLength,e.number,!0)}return e.arr=Array.from(t),e.arr}function t(t){if(!t)return;if(!t.length)return;let n=Object.prototype.toString,o=[];for(let L=0;L<t.length;L++)if(typeName=n.call(t[L]).toLowerCase().split(' ')[1],0<=typeName.indexOf('number')){let n=t[L],u=n.toString();if(parseInt(u)==parseFloat(u))n>=0?n<256?(o.push(r),o=e({arr:o,byteLength:1,number:n,type:'8'})):n>255&&n<65536?(o.push(a),o=e({arr:o,byteLength:2,number:n,type:'16'})):n>65535&&n<4294967296?(o.push(g),o=e({arr:o,byteLength:4,number:n,type:'32'})):(o.push(h),o=e({arr:o,byteLength:8,number:n,type:'64'})):n>=-128?(o.push(b),o=e({arr:o,byteLength:1,number:n,type:'8'})):n<-128&&n>=-32768?(o.push(f),o=e({arr:o,byteLength:2,number:n,type:'16'})):n<-32768&&n>=-2147483648?(o.push(i),o=e({arr:o,byteLength:4,number:n,type:'32'})):(o.push(y),o=e({arr:o,byteLength:8,number:n,type:'64'}));else{let t=n.toString().length-1;t>5?(o.push(l),o=e({arr:o,byteLength:8,number:n,type:'d'})):(o.push(p),o=e({arr:o,byteLength:4,number:n,type:'f'}))}}else if(0<=typeName.indexOf('bigint'))o.push(h),o=e({arr:o,byteLength:8,number:t[L],type:'64'});else if(0<=typeName.indexOf('uint8array')){o.push(u),o=e({arr:o,byteLength:4,number:t[L].length,type:'32'});for(let e of t[L])o.push(e)}else if(0<=typeName.indexOf('string')){o.push(s);let n=(new TextEncoder).encode(t[L]);o=e({arr:o,byteLength:4,number:n.length+1,type:'32'});for(let e of n)o.push(e);o.push(0)}return new Uint8Array(o).buffer}function n(e,t,n,o,L){if(!e)return;if(!e.byteLength)return;let c,m=[],d=e=>n?{}:[];try{c=new DataView(e)}catch(e){return d()}let U=0;for(;U<e.byteLength;){let t=c.getUint8(U);if(t>247&&(t-=256),m.length&&o)break;switch(t){case r:case b:if(++U,e.byteLength-U<1)return d();if(L?m.push({data:t==r?c.getUint8(U):c.getInt8(U),offset:U,length:1}):m.push(t==r?c.getUint8(U):c.getInt8(U)),++U,o)break;break;case a:case f:if(++U,e.byteLength-U<2)return d();L?m.push({data:t==a?c.getUint16(U,!0):c.getInt16(U,!0),offset:U,length:2}):m.push(t==a?c.getUint16(U,!0):c.getInt16(U,!0)),U+=2;break;case g:case i:if(++U,e.byteLength-U<4)return d();L?m.push({data:t==g?c.getUint32(U,!0):c.getInt32(U,!0),offset:U,length:4}):m.push(t==g?c.getUint32(U,!0):c.getInt32(U,!0)),U+=4;break;case h:case y:if(++U,e.byteLength-U<8)return d();L?m.push({data:t==h?c.getBigUint64(U,!0):c.getBigInt64(U,!0),offset:U,length:8}):m.push(t==h?c.getBigUint64(U,!0):c.getBigInt64(U,!0)),U+=8;break;case p:case l:if(++U,e.byteLength-U<(t==p?4:8))return d();L?m.push({data:m.push(t==p?c.getFloat32(U,!0):c.getFloat64(U,!0)),offset:U,length:t==p?4:8}):m.push(t==p?c.getFloat32(U,!0):c.getFloat64(U,!0)),U+=t==p?4:8;break;case u:if(++U,e.byteLength-U<4)return d();let n=c.getUint32(U,!0);if(U+=4,e.byteLength-U<n)return d();if(L&&n+U>e.byteLength){m.push({offset:U,length:n});break}let w=new Uint8Array(n);for(let e=0;e<n;e++)w[e]=c.getUint8(U++);L?m.push({data:w,offset:U-n-1,length:n}):m.push(w);break;case s:if(++U,e.byteLength-U<4)return d();let k=c.getUint32(U,!0)-1;if(U+=4,e.byteLength-U<k)return d();let B=new Uint8Array(k);for(let e=0;e<k;e++)B[e]=c.getUint8(U++);B=(new TextDecoder).decode(B),++U,L?m.push({data:B,offset:U-k-1,length:k}):m.push(B)}}if(!n)return m;if(!n.length)return m;let w={};for(let e=0;e<n.length;e++)e>m.length-1?w[n[e]]=null:w[n[e]]=m[e];return w}const r=1,a=2,g=4,h=8,u=10,s=9,b=-1,f=-2,i=-4,y=-8,p=6,l=7;'object'==typeof window?(window.createArrayBuffer=t,window.decodeArrayBuffer=n):module.exports={createArrayBuffer:t,decodeArrayBuffer:n}})(); </script> <script> (function(){let e,r,t,l=(e,r)=>r>>>e|r<<32-e,o=(e,r,t)=>e&r^~e&t,n=(e,r,t)=>e&r^e&t^r&t,f=e=>l(2,e)^l(13,e)^l(22,e),a=e=>l(6,e)^l(11,e)^l(25,e),w=e=>l(7,e)^l(18,e)^e>>>3,A=e=>l(17,e)^l(19,e)^e>>>10,y=(e,r)=>e[15&r]+=A(e[r+14&15])+e[r+9&15]+w(e[r+1&15]),c=new Array(1116352408,1899447441,3049323471,3921009573,961987163,1508970993,2453635748,2870763221,3624381080,310598401,607225278,1426881987,1925078388,2162078206,2614888103,3248222580,3835390401,4022224774,264347078,604807628,770255983,1249150122,1555081692,1996064986,2554220882,2821834349,2952996808,3210313671,3336571891,3584528711,113926993,338241895,666307205,773529912,1294757372,1396182291,1695183700,1986661051,2177026350,2456956037,2730485921,2820302411,3259730800,3345764771,3516065817,3600352804,4094571909,275423344,430227734,506948616,659060556,883997877,958139571,1322822218,1537002063,1747873779,1955562222,2024104815,2227730452,2361852424,2428436474,2756734187,3204031479,3329325298),d='0123456789abcdef',u=(e,r)=>{let t=(65535&e)+(65535&r),l=(e>>16)+(r>>16)+(t>>16);return l<<16|65535&t},h=()=>{e=new Array(8);r=new Array(2);t=new Array(64);r[0]=r[1]=0;e[0]=1779033703;e[1]=3144134277;e[2]=1013904242;e[3]=2773480762;e[4]=1359893119;e[5]=2600822924;e[6]=528734635;e[7]=1541459225},i=()=>{let r,l,w,A,d,h,i,g,s,b,p=new Array(16);r=e[0];l=e[1];w=e[2];A=e[3];d=e[4];h=e[5];i=e[6];g=e[7];for(let e=0;e<16;e++)p[e]=t[3+(e<<2)]|t[2+(e<<2)]<<8|t[1+(e<<2)]<<16|t[e<<2]<<24;for(let e=0;e<64;e++){s=g+a(d)+o(d,h,i)+c[e];s+=e<16?p[e]:y(p,e);b=f(r)+n(r,l,w);g=i;i=h;h=d;d=u(A,s);A=w;w=l;l=r;r=u(s,b)}e[0]+=r;e[1]+=l;e[2]+=w;e[3]+=A;e[4]+=d;e[5]+=h;e[6]+=i;e[7]+=g},g=(e,l)=>{let o,n,f=0;n=r[0]>>3&63;let a=63&l;(r[0]+=l<<3)<l<<3&&r[1]++;r[1]+=l>>29;for(o=0;o+63<l;o+=64){for(let r=n;r<64;r++)t[r]=e.charCodeAt(f++);i();n=0}for(let r=0;r<a;r++)t[r]=e.charCodeAt(f++)},s=()=>{let e=r[0]>>3&63;t[e++]=128;if(e<=56)for(let r=e;r<56;r++)t[r]=0;else{for(let r=e;r<64;r++)t[r]=0;i();for(let e=0;e<56;e++)t[e]=0}t[56]=r[1]>>>24&255;t[57]=r[1]>>>16&255;t[58]=r[1]>>>8&255;t[59]=255&r[1];t[60]=r[0]>>>24&255;t[61]=r[0]>>>16&255;t[62]=r[0]>>>8&255;t[63]=255&r[0];i()},b=()=>{let r=0,t=new Array(32);for(let l=0;l<8;l++){t[r++]=e[l]>>>24&255;t[r++]=e[l]>>>16&255;t[r++]=e[l]>>>8&255;t[r++]=255&e[l]}return t},p=()=>{let r=new String;for(let t=0;t<8;t++)for(let l=28;l>=0;l-=4)r+=d.charAt(e[t]>>>l&15);return r},C=(e,r,t)=>{h();g(e,e.length);s();return r?b():p()};'object'==typeof window?window.getHash=C:module.exports=C})(); </script> <script> (e=>{let t=window,o=e=>document.getElementById(e),n=t.showMsg=((e,t)=>{t=t||3e3;let n=o('msg');try{clearTimeout(n.t)}catch(e){}n.innerHTML=e,n.className+=' msgShow',n.t=setTimeout(()=>{n.className='msg'},t)}),c=!0,r={cn:{ssid:'WiFi名称',wifiPwd:'WiFi密码',user:'用户名',userPwd:'密码',websocketDomain:'WebSocket主机',websocketPort:'WebSocket端口',websocketPath:'WebSocket路径',nickname:'昵称',token:'令牌',setArguments:'设置参数',reboot:'重启',rebootNow:'立即重启',rollback:'回滚固件',deepSleep:'睡眠10分钟',txtError:'错误',txtSet:'设置参数成功',txtDelayReboot:'设备将会在3秒后重启',txtRollback:'回滚操作成功',txtDeepSleep:'设备即将睡眠',txtConnected:'已连接',txtDisconnected:'已断开',txtInvalid:'不合法'},en:{ssid:'WiFi SSID',wifiPwd:'WiFi password',user:'User name',userPwd:'password',websocketDomain:'WebSocket host',websocketPort:'WebSocket port',websocketPath:'WebSocket path',nickname:'Nickname',token:'Token',setArguments:'Submit',reboot:'Reboot',rebootNow:'Reboot immediately',rollback:'Rollback firmware',deepSleep:'Deep sleep 10 minutes',txtError:'Error',txtSet:'Arguments set',txtDelayReboot:'Will reboot in 3 seconds',txtRollback:'Rollback OK',txtDeepSleep:'Will into deep sleep in 3 seconds',txtConnected:'Connected',txtDisconnected:'Disconnected',txtInvalid:' Invalid'}};(e=>{let t=navigator.language;t.length&&(t=t.substring(0,2).toLowerCase(),t.startsWith('en')&&(c=!1));let o=document.getElementsByTagName('a');for(let e of o)e.href&&''!=e.href||(e.href='javascript:void(0);'),e.innerText=c?r.cn[e.id]:r.en[e.id];o=document.getElementsByTagName('input');for(let e of o)'text'!=e.type&&'password'!=e.type||(e.placeholder=c?r.cn[e.id]:r.en[e.id])})();let l=t.connectWebsocket=(e=>{try{t.webSocket=new WebSocket('ws://'+t.location.host+':80'),t.webSocket.binaryType='arraybuffer',t.webSocket.onerror=(e=>{n((c?r.cn.txtError:r.en.txtError)+': '+JSON.stringify(e)),setTimeout(()=>{l()},1e3)}),t.webSocket.onmessage=(e=>{if(e.data&&e.data.byteLength>=2){let t=decodeArrayBuffer(e.data);t&&t.length&&(0==t[0]?n(c?r.cn.txtSet:r.en.txtSet):1==t[0]?n(c?r.cn.txtDelayReboot:r.en.txtDelayReboot):136==t[0]?n(c?r.cn.txtRollback:r.en.rollback):137==t[0]?n(c?r.cn.txtDeepSleep:r.en.deepSleep):t[0])}}),t.webSocket.onopen=function(){n(c?r.cn.txtConnected:r.en.txtConnected)},t.webSocket.onclose=(e=>{n(c?r.cn.txtDisconnected:r.en.txtDisconnected),setTimeout(()=>{l()},1e3)})}catch(e){setTimeout(()=>{l()},1e3)}});t.connectWebsocket();let i=t.launch=function(e){t.webSocket.send(createArrayBuffer(e))};o('setArguments').onclick=(e=>{let t=e=>{n(e+(c?r.cn.txtInvalid:r.en.txtInvalid))},l=o('nickname').value,a=o('ssid').value;if(!a.trim().length)return void t(c?r.cn.ssid:r.en.ssid);let s=o('wifiPwd').value;if(!s.trim().length)return void t(c?r.cn.wifiPwd:r.en.wifiPwd);let d=o('user').value;if(!d.trim().length)return void t(c?r.cn.user:r.en.user);let b=o('userPwd').value;if(!b.trim().length)return void t(c?r.cn.userPwd:r.en.userPwd);let u=o('websocketDomain').value;if(!u.trim().length)return void t(c?r.cn.websocketDomain:r.en.websocketDomain);let k=o('websocketPort').value;k=k.replace(/\s/g,''),k=k.length?k:'80',k=parseInt(k);let w=o('websocketPath').value;w=w.replace(/\s/g,''),w=w.length?w:'/';let m=o('token').value;i([0,l,a,s,new Uint8Array(getHash(d,!0)),new Uint8Array(getHash(b,!0)),u,k,w,m])}),o('reboot').onclick=(e=>{i([1])}),o('rebootNow').onclick=(e=>{i([2])}),o('rollback').onclick=(e=>{i([136])}),o('deepSleep').onclick=(e=>{i([137])}),(e=>{let t=document.getElementsByTagName('input');for(let e of t)'text'!=e.type&&'password'!=e.type||(e.onkeyup=function(e){localStorage[this.id]=this.value});for(let e in localStorage){let t=o(e);t&&(t.value=localStorage[e])}})()})(); </script></body></html>)";
//apindex

    /**
     * @brief default gateway of AP mode
     * AP模式的默认网关
     *
     * @note you could modify this ip address
     * 你可以自己修改这个ip地址
     */
    IPAddress *apIP = new IPAddress(192, 168, 8, 1);

    /**
     * @brief callback for STA mode after esp32 got ip address
     * STA模式esp32获得ip地址的回调函数
     */
    WiFiEventSysCb fnWiFiSTAGotIP = nullptr;

    /**
     * @brief callback when disconnected from access point
     * 断开wifi连接之后的回调函数
     */
    WiFiEventSysCb fnWiFiSTADissconnected = nullptr;

    /**
     * @brief the time that system power on(connected to remote server)
     * unix epoch timestamp
     *
     * 系统启动时间(连接到远程服务器后)
     * unix时间戳
     */
    uint64_t systemPowerOnTime = 0;

    /**
     * @brief incomming command from local area another device
     * from esp now
     *
     * 从本地其他设备使用esp now 发送来的命令
     *
     * @attention this function is still developing
     * 这个功能正在开发中
     *
     * @param data payload 负载数据
     * @param length length of payload 负载数据的长度
     */
    void commandFromEspNow(uint8_t *data, uint32_t length);

    /**
     * @brief transfer data from esp now to websocket handler
     * 把esp now发送来的数据传送到websocket去处理
     *
     * @attention this function is still developing
     * 这个功能正在开发中
     *
     * @param data payload 数据负载
     * @param length length of payload 数据负载的长度
     * @param client instance of websocket client, websocket 客户端实例
     * @param event websocket event, websocket 事件
     */
    void routeDataToWebsocketHandler(uint8_t *data,
                                     uint32_t length,
                                     myWebSocket::WebSocketClient *client = nullptr,
                                     myWebSocket::WebSocketEvents event = myWebSocket::TYPE_UNKNOWN);

    /**
     * @brief remove earliest hash that had been used by authorization
     * 清理掉最早的一个用于认证的一次性密匙
     */
    void removeEarliestHash();

    /**
     * @brief indicate that softAP if started
     * 指示AP热点是否已开启
     */
    bool isAPStarted = false;

    /**
     * @brief AP will NOT enable if this set to true
     * 如果此值设置为true，AP不会自动开启
     *
     */
    bool doNotEnableAP = false;

    /**
     * @brief mark current firmware is valid
     * 标记当前运行的固件是有效的
     */
    void markNewFirmwareIsValid();

    /**
     * @brief close softAP
     * 关闭AP热点
     */
    inline void closeAP()
    {
        // if (!this->isAPStarted)
        // {
        //     return;
        // }
        this->isAPStarted = false;
        delete this->apServer;
        this->apServer = nullptr;
        WiFi.enableAP(false);
    }

    /**
     * @brief start soft AP
     * 开启AP热点
     */
    void startAP();

    /**
     * @brief push message to administrator
     * 推送消息给管理员
     *
     * @param msg message 消息
     */
    bool sendMessageToClient(const Element &msg);

    /**
     * @brief generate disposable hash for authorization
     * 生成用于认证的一次性密匙
     *
     * @return OneTimeAuthorization* timestamp and hash 时间戳和数字摘要
     */
    OneTimeAuthorization *generateOneTimeAuthorization();

    /**
     * @brief convert a short command to long command
     * 把短命令转换为长命令
     *
     * @param command short command 短命令
     * @param confirm need confirm 是否需要确认
     * @return uint64_t long command 长命令
     */
    inline uint64_t patchLongCommandFixedHeader(uint8_t command, bool confirm = false)
    {
        uint64_t c = (confirm ? 0xC000000000000000ULL : 0x8000000000000000ULL) | ((uint64_t)(command));
        return c;
    }

    /**
     * @brief execute provider on another deivce
     * 执行另一个设备上的provider
     *
     * @param frinedID id of target 对方的id
     * @param providerID id of target provider 对方的provider的id
     * @param confirm need confirm or not 是否需要确认
     * @return true success 成功
     * @return false failed 失败
     */
    inline bool callFriendProvider(String frinedID, uint16_t providerID, bool confirm = false)
    {
        return confirm ? (this->_sendBundle(frinedID,
                                            this->patchLongCommandFixedHeader(CMD_EXECUTE_COMMAND, true),
                                            providerID))
                       : (this->_sendBundle(frinedID,
                                            (uint8_t)(CMD_EXECUTE_COMMAND),
                                            providerID));
    }

    /**
     * @brief execute provider on another deivce directly use
     * user built request container
     *
     * 用 用户自定义的容器执行另一个设备上的provider
     *
     * @param frinedID target id 对方id
     * @param request container 容器
     * @return true success 成功
     * @return false failed 失败
     */
    inline bool callFriendProvider(String frinedID, Elements *request)
    {
        return this->_sendBundle(frinedID, 0, 0, request);
    }

    /**
     * @brief indicate time that administrator
     * last time online
     *
     * 指示上次管理员在线的时间
     *
     */
    uint64_t lastTimeAdminOnline = 0;

    /**
     * @brief buffer of all providers
     * 全部provider的缓存
     */
    uint8_t *bufferProviders = nullptr;

    /**
     * @brief buffer length of all providers
     * 全部provider的缓存的长度
     */
    uint32_t bufferProvidersLength = 0;

    /**
     * @brief set wifi connection status
     * 设置wifi连接的状态
     *
     * @param isConnected connected 是否已连接
     */
    inline void setWiFiStatus(bool isConnected = true) { this->isWifiConnected = isConnected ? 0xffu : 0; }

    /**
     * @brief refresh data in ram
     * ensure those variables
     * always provide right value
     *
     * 刷新内存中的数据
     * 确保这些变量
     * 提供正确的值
     *
     */
    void refreshData();

    /**
     * @brief get administrator user name
     * 获取管理员用户名
     *
     * @return Element administrator user name 管理员用户名
     */
    inline Element getUserName() { return this->userName; }

    /**
     * @brief get administrator password
     * 获取管理员密码
     *
     * @return Element administrator password 管理员密码
     */
    inline Element getNickname() { return this->nickname; }

    /**
     * @brief get universal id of current device
     * 获取当前设备的id
     *
     * @return Element* id
     */
    inline const Element &getUniversalID() { return this->UniversalID; }

    /**
     * @brief to authorize a request legal or not
     * 认证一个请求是否合法
     *
     * @param remoteHash hash from remote 远程设备发送来的数字摘要
     * @param timestamp unix epoch timestamp from remote 远程设备发送来的unix时间戳
     * @param isAdmin return user is admin or not 返回认证的用户是否是管理员
     * @return true authorized 已通过认证
     * @return false unauthorized 未通过认证
     */
    bool authorize(Element *remoteHash, Element *timestamp, bool *isAdmin);

    /**
     * @brief buffer of other users
     * 其他用户的缓存
     */
    uint8_t *usersBuffer = nullptr;

    /**
     * @brief buffer length of other users
     * 其他用户的缓存的长度
     */
    uint32_t lengthOfUsersBuffer = 0;

    /**
     * @brief get websocket client connected for remote
     * 获取用于连接远程服务器的websocket客户端
     *
     * @return myWebSocket::WebSocketClient* websocket client instance, websocket客户端实例
     */
    inline myWebSocket::WebSocketClient *getWebsocketClient() { return this->websocketClient; }

    /**
     * @brief add custom provider
     * 添加自定义provider
     *
     * @note user could use this function to add custom
     * provider at app setup
     *
     * 用户可以在用户应用入口函数中使用此函数添加自定义provider
     *
     * @param cb callback 回调函数
     * @param name name for human read 名字，给人看的
     * @param settings provider settings, provider设置
     * refer to provider.h enum
     * 枚举在 provider.h 里
     *
     * @param lengthOfArguments indicate how many arguments of current provider
     * 当前添加的provider 需要几个参数
     * @param customID custom id for other purpose 用于其他用途的id
     */
    inline void createProvider(ProviderCallback cb,
                               String name,
                               uint8_t settings = 0,
                               uint8_t lengthOfArguments = 0,
                               uint64_t customID = 0)
    {
        Provider *p = new Provider(this->providers->size(), cb, name, settings, lengthOfArguments);
        p->customID = customID;
        this->providers->push_back(p);
    }

    /**
     * @brief add custom provider
     * 添加自定义provider
     *
     * @param builtIn indicate current provier is built-in or not
     * 指示当前provider是否是内建的
     *
     * @param cb callback 回调函数
     * @param name name for human read 名字，给人看的
     * @param settings provider settings, provider设置
     * refer to provider.h enum
     * 枚举在 provider.h 里
     *
     * @param lengthOfArguments indicate how many arguments of current provider
     * 当前添加的provider 需要几个参数
     *
     *
     */
    inline void createProvider(bool builtIn,
                               ProviderCallback cb,
                               String name,
                               uint8_t settings = 0,
                               uint8_t lengthOfArguments = 0)
    {
        Provider *p = new Provider(this->providers->size(), cb, name, settings, lengthOfArguments);
        p->isBuiltIn = builtIn;
        this->providers->push_back(p);
    }

    /**
     * @brief default constructor
     * 默认构造函数
     *
     * @param inititlizeSerial enable hard serial or not 是否使能硬件串口
     */
    inline GlobalManager(bool inititlizeSerial = true)
    {
        // initialize serial
        if (inititlizeSerial)
            Serial.begin(115200);
    }

    inline ~GlobalManager() {}

    /**
     * @brief initialize file system
     * 初始化文件系统
     */
    inline void beginFS() { MyFS::myfsInit(); }

    /**
     * @brief initialize main database
     * 初始化主数据库
     */
    inline void beginMainDB() { db.begin(); }

    /**
     * @brief initialize other databases
     * 初始化其他数据库
     */
    inline void beginOtherDB()
    {
        dbUser.begin();
        dbApp.begin();
    }

    /**
     * @brief generate universal id by MAC
     * 用物理地址生成ID
     */
    inline void beginUniversalID()
    {
        uint8_t id[6] = {0};
        esp_efuse_mac_get_default(id);
        uint8_t buf[32] = {0};
        mycrypto::SHA::sha256(id, 6, buf);
        this->UniversalID(buf,32);
    }

    inline void beginLogCommandAndAdminID(){
        this->eLogCommand = CMD_LOG;
        this->eAdminIDStr = mycrypto::SHA::sha256(this->userName.getHex() + this->password.getHex() + "admin");
    }

    /**
     * @brief load administrator user name and password
     * 加载管理员用户名和密码
     */
    inline void beginUserInfo()
    {
        this->userName = db("userName");
        this->password = db("password");
    }

    /**
     * @brief start some basic operations
     * load user,
     * build buffer,
     * load nickname ...
     *
     * 执行一些基础操作
     * 加载用户信息、建立缓存、加载昵称 ...
     */
    void initializeBasicInformation();

    /**
     * @brief set user custom callback for AP mode websocket server
     * 设置用户自定义的用于AP模式websocket服务器的回调函数
     *
     * @param cb callback 回调函数
     */
    inline void setExtraLocalWebsocketCallback(WebSocketCallback cb) { this->extraAPWebsocketCallback = cb; }

    /**
     * @brief set user custom callback for STA mode websocket client callback
     * 设置用户自定义的用于STA模式websocket客户端的回调函数
     *
     * @param cb callback 回调函数
     */
    inline void setExtraRemoteWebsocketCallback(WebSocketCallback cb) { this->extraWifiWebsocketCallback = cb; }

    /**
     * @brief load everything
     * 加载所有
     *
     * @param apCB user custom callback for AP mode websocket server
     * 用户自定义的用于AP模式websocket服务器的回调函数
     *
     * @param wifiCB user custom callback for STA mode websocket client callback
     * 用户自定义的用于STA模式websocket客户端的回调函数
     */
    void beginAll(WebSocketCallback apCB = nullptr,  // extra websocket callback for local
                  WebSocketCallback wifiCB = nullptr // extra websocket callback for remote
    );

    /**
     * @brief Main loop of whole system
     * 整个系统的主循环
     */
    void loop();

    /**
     * @brief synchronize real time from remote server
     * 从远程服务器同步实时时间
     *
     * @param t unix epoch timestamp, unix时间戳
     */
    void syncTime(uint64_t t);

    /**
     * @brief check if wifi enabled
     * 检查是否使用wifi
     *
     * @return true wifi is enabled; wifi已经启用
     * @return false wifi is disabled; wifi已禁用
     */
    inline bool wifiEnabled()
    {
        return (bool)(this->isWifiEnabled);
    }

    /**
     * @brief show if OTA update running
     * 显示OTA升级是否正在运行
     *
     * @return true OTA update is running; OTA升级正在运行
     * @return false OTA update is not running; OTA升级未在运行
     */
    inline bool isOTAUpdateRunning()
    {
        return (bool)(this->ota);
    }

    /**
     * @brief show connection status of wifi
     * 显示wifi连接状态
     *
     * @return true wifi connected; wifi已连接
     * @return false wifi 已断开; wifi已断开
     */
    inline bool wifiConnected()
    {
        return (bool)(this->isWifiConnected);
    }

    /**
     * @brief set or get server offline times
     * 获取或设置服务器被检测到掉线的次数
     *
     * @param times times will be set as this number if postive value provided
     * 如果提供一个正整数，则服务器掉线次数会被设置为此值
     *
     * @return uint16_t the times detected server offline; 服务器被检测到掉线的次数
     */
    inline uint16_t serverOfflineTimes(int32_t times = -2)
    {
        if (times > 0)
        {
            this->remoteServerOfflineDetectedTimes = times;
        }
        return this->remoteServerOfflineDetectedTimes;
    }

    /**
     * @brief get or set server status
     * 获取或设置服务器状态
     *
     * @param serverStatus mark server status manually; 手动标记服务器状态
     * 0 == get status; negative number == offline; positive number == online
     *
     * @note if positive number provided, will also reset server offline detected times value
     * 如果提供了一个正数，会顺便重置检测到服务器掉线的次数
     *
     * if negative number provided, will also set offline to 1 or increase offline times if the value
     * already had a positive number
     * 如果提供了一个负数，会同时设置掉线次数为1，如果掉线次数已经为一个正数，会给它+1
     *
     * @return true server online; 服务器在线
     * @return false server offline; 服务器掉线
     */
    inline bool serverOnline(uint8_t serverStatus = 0)
    {
        if (serverStatus)
        {
            if (serverStatus > 0)
            {
                this->isServerOnline = 0xffu;
                this->remoteServerOfflineDetectedTimes = 0;
            }
            else
            {
                this->isServerOnline = 0;
                if (this->remoteServerOfflineDetectedTimes > 0)
                {
                    this->remoteServerOfflineDetectedTimes++;
                }
                else
                {
                    this->remoteServerOfflineDetectedTimes = 1;
                }
            }
        }
        return this->isServerOnline;
    }

    inline uint64_t getRemoteWebsocketConnectedTime()
    {
        return this->remoteWebsocketConnectedTimestamp;
    }

    /**
     * @brief remove wifi ssid and password from database
     * 从数据库清除wifi连接信息
     */
    void resetWifiInfo();

    /**
     * @brief reboot esp32 in a timeout
     * 在一段时间之后复位esp32
     *
     * @param timeout timeout 超时时间
     */
    inline void delayReboot(uint32_t timeout = 3000U)
    {
        setTimeout([]()
                   { ESP.restart(); },
                   timeout);
    }

    /**
     * @brief indicate that esp32 send data from hard serial
     * to admin through websocket
     *
     * 指示esp32是否将硬件串口的数据通过websocket转发给管理员
     *
     */
    uint8_t isSerialDataLoopBack = 0xffu;

    inline void enableSerialDataLoopBack(bool enable)
    {
        this->isSerialDataLoopBack = enable ? 0xffu : 0x00u;
        this->setSerialRecvCb();
    }

    void setSerialRecvCb();

    /**
     * @brief sometimes, when you want to use BT or BLE
     * you wouldn't like to enable wifi
     * then you could call this in app setup
     * attention attached
     *
     * 有时候如果你使用 BT 或 BLE
     * 你不想打开 wifi
     * 那么你可以在 app 的 setup 函数里调用这个函数
     * 请阅读注意事项
     *
     * @param enable enable wifi 启动wifi
     * @param enablePin the pin to detect enable wifi or not
     * 检测是否启动wifi的引脚
     *
     * @return true enable wifi at boot 本次启动 启用wifi功能;
     * @return false your set value or detected result 你设置的选项或检测引脚的结果
     *
     * @note
     * example 例子:
     *
     * App::setup() {
     *
     *      bool wifiEnabled = global->enableWifi(false, 23);
     *
     *      if ( wifiEnabled ) {
     *          //current boot will use wifi to connect to AP
     *          //you can do OTA update or other things
     *          //本次启动将会使用wifi连接到AP
     *          //你可以做OTA升级或其他工作
     *      } else {
     *          //current boot will NOT use wifi
     *          //you could use BT or BLE
     *          //本次启动 不会 使用wifi
     *          //你可以使用BT或BLE
     *      }
     * }
     *
     * @attention
     * when you use this feature, you need to confirm new firmware valid manually
     * use global->markNewFirmwareIsValid();
     *
     * 当你使用这个功能时，你需要手动确认新固件的有效性
     * 使用 global->markNewFirmwareIsValid();
     */
    inline bool enableWifi(bool enable, uint8_t enablePin = 0, int defaultLevel = HIGH)
    {
        if (enablePin)
        {
            pinMode(enablePin, INPUT);
            if (digitalRead(enablePin) == defaultLevel)
            {
                delay(50);
                if (digitalRead(enablePin) == defaultLevel)
                {
                    this->isWifiEnabled = 0xffu;
                    return true;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            this->isWifiEnabled = enable ? 0xffu : 0x00u;
            return enable;
        }
    }

    /**
     * @brief Reset RX buffer size for serial0
     * you should call this if your target will send large data
     * through UART to esp32
     *
     * 重新设定Serial0 的 RX 缓冲区大小
     * 如果连接的目标会通过串口发送大量数据
     * 你需要调用这个函数
     *
     * @param newSize new size for RX buffer; RX缓冲区的新的大小
     * @param buadRate default buadrate; 默认波特率
     *
     * @return true success; 成功
     * @return false failed; 失败
     */
    inline bool setHWSerial0BufferSize(uint32_t newSize, uint32_t buadrate = 115200)
    {
        uint32_t br = Serial.baudRate();
        br = br != buadrate ? buadrate : br;
        Serial.end();
        bool result = false;
        if (Serial.setRxBufferSize(newSize))
        {
            result = true;
        }
        Serial.begin(br);
        return result;
    }

    uint32_t globalTask = 0ULL;
};

/**
 * @brief global manager global object
 * use pointer to distinguish
 *
 * 全局管理器的全局对象
 * 使用指针用以区分
 */
extern GlobalManager *global;

#endif