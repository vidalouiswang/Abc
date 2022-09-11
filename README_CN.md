# Abc

## 一个为新手打造的 esp32 基础库

### 目前，使用该库构建的 ESP32 固件已经稳定运行了数月，可以正式使用了。

### 备注: 从主分支克隆代码。

## 简介

这个库和乐鑫官方 esp32-arduino 2.0.3 库一起使用。
仅在 esp32 完成测试。
这个库是为了那些想要自己编写代码，但是却又没有太多基础的新手打造的，这个库可以成为你学习 esp32 的起点，你不仅可以使用 esp32-arduino 的所有功能，也能使用 esp-idf 的功能，可以由浅入深的学习如何开发 ESP32。

### 功能

* 内置了方便的 OTA 升级功能，无需任何一行代码即可使用。
* 存在故障的固件可以自动回滚，再也不用担心与安装到电饭锅里的 ESP32 失联( esp-idf 有这个功能，但是 esp32-arduino 库把这个功能关闭了，本库无需修改自动下载的SDK即可实现这个功能)。
* 非常简单的方法就可以存储和传输整数、字符串、二进制数据而无需再使用其他格式(比如 json )。
* 内置了便于使用的内存数据库，使用 LittleFS 进行存储( esp-idf 有这个功能，但是根据官方文档没有磨损均衡)。
* 稳定且快速的 Websocket 实现。
* 一行代码就能用的 SHA1，SHA256 数字摘要功能，基于ESP32硬件加速实现，可使用多种不同种类的数据作为输入。
* 一行代码就能用的 AES-256-CBC 加密与解密功能，基于ESP32硬件加速实现。
* 更多功能正在开发中...

以上功能均是为新手开发的，使用方便扩展性强，高手请忽略。

# 如何使用

### 基础配置演示视频 https://www.bilibili.com/video/BV14d4y1X7Xs

开发环境: 安装了 Platform IO 的 Visual studio code.

首先你需要有一台 VPS，安装了 node.js 16。
或者你也可以用你的电脑或 NAS 当做服务器。

### 第一步：克隆代码

```console
git clone https://github.com/vidalouiswang/Abc.git
```

### 第二步：配置服务器

1. 使用SSH连接你的VPS

2. 安装 包管理器 , 假设你的系统是 Ubuntu

```console
sudo apt-get install npm
```

3. 安装n模块
```console
sudo npm install -g n
```

4. 安装 Node.js
```console
sudo n stable
```

5. 安装 pm2
```console
sudo npm install -g pm2
```

现在你的node.js应该已经处于最新稳定版了，可以输入如下命令查看

```console
node -v
```

如果出现 "v16.x" 证明安装已经完成了，如果依然显示旧版本，比如 "v8.x"，你可以输入如下命令

```console
hash -r
```

6. 如果你的代码克隆到你的个人电脑，你需要上传
/server/
    - index.html
    - ab.js
    - hash.js
    - create.js
    - iot.js

这五个文件到你的服务器，在SSH中按如下步骤操作

```console
cd ~/
```
```console
mkdir server
```
```console
cd server
```

然后把上面的五个文件使用你喜欢的方式上传到server文件夹下

如果你直接将代码克隆到服务器则这一步可以省略，直接
```console
cd Abc/server
```

7. 安装基础组件
```console
sudo npm install ws
```

8. 开启服务器
```console
sudo pm2 start iot.js
```

服务器已经配置完毕，默认端口为 12345， 可以自己修改，配置在 "globalConfig.json" 中

也可以直接在代码文件 iot.js 最底部修改

如果修改了端口记得重新启动服务器
```console
sudo pm2 restart iot.js
```

请注意在防火墙放行相应端口

### 第三步：本地使用

1. 使用 Platform IO 打开根目录。
2. 定位到路径 /src/app。 
3. 然后编辑 app.h 和 app.cpp，编程方式和 arduino 一样，在函数 setup 和 loop 中添加代码即可。
4. 编译上传固件(首次使用最好先擦除flash)。
5. 使用电脑或手机搜索esp32热点，然后连接。
6. 打开浏览器，进入 http://192.168.8.1。
7. 根据提示设置参数然后点击重启。
8. 打开 http://你的服务器ip或域名:端口/。
9. 使用在第7步中设置的用户明和密码登录。
10. 你可以看到在线的设备，点击 "@" 可以访问内置Provider。

其他功能请参考示例，所有示例均有详细的中英双语注释，也可关注本人B站账号，会不定期发布新的视频。

### 参考头文件查看基础信息与各种配置，在 /lib/config/config.h。

### LittleFS 额外组件自动选择功能还未在Windows环境测试过，但是它应该可以工作，目前在macOS 12.5工作正常。

### 更多教程和文档正在编写中。

### 代码注释状态(全部的、详细的双语注释，其他文件也有注释，一般都是开发时用的简单英文注释):

* 本地:

    * 头文件:

        * 几乎所有的头文件
        * 示例的所有头文件

    * 源文件:

        * /lib/arraybuffer/arraybuffer.cpp
        * /src/main/main.cpp
        * 示例的所有源文件

* 服务器

    * /server/iot.js
    * /server/create.js

# 感谢

感谢乐鑫的全体工作人员，让我们拥有了便宜好用的ESP32以及其他芯片。
感谢Node.js的全体开发者，让我们拥有了便捷好用的JavaScript运行时。
感谢VSCode和Platform IO的全体开发者，让我们拥有了好用的集成开发环境。

[Espressif](https://github.com/espressif)

[Node.js](https://github.com/nodejs)

[Visual Studio Code](https://github.com/microsoft/vscode)

[Platform IO](https://github.com/platformio)

[ws](https://github.com/websockets/ws)

[Brix](https://github.com/brix/crypto-js)

[Felix Biego](https://github.com/fbiego/ESP32Time)

# 协议

(GNU General Public License v3.0 License)

Copyright 2022, Vida Wang <github@vida.wang>

孩子是人类的未来，但是现在全世界仍然有许多孩子饱受饥饿，如果你是个善良的人、认可我的代码，请捐助联合国儿童基金会，谢谢。
https://unicef.cn
