# Abc

### 一个为新手打造的 esp32 基础库(Beta)

这个库和乐鑫官方 esp32-arduino 2.0.3 库一起使用。
仅在 esp32 完成测试。

这个库是为了那些想要自己编写代码，但是却又没有太多基础的新手打造的，这个库可以成为你学习 esp32 的起点。

# 如何使用

首先你需要有一台 VPS，安装了 node.js 16。
或者你也可以用你的电脑或 NAS 当做服务器。

### 克隆

```console
git clone https://github.com/vidalouiswang/Abc.git
```

### 上传

    /server/
        - pro/
            - index.html
        - ab.js
        - hash.js
        - create.js
        - iot.js

这五个文件到你的服务器。
(这5个文件应该放在同一个目录，文件树只是告诉你应该上传哪5个文件)

### 然后执行命令

```console
npm install ws
```

```console
npm install pm2
```

```console
pm2 start iot.js
```

### 本地

1. 使用 Platform IO 打开根目录。
2. 定位到 /src/app。 
3. 然后编辑 app.h 和 app.cpp。
4. 编译上传固件(最好先格式化flash)。
5. 使用电脑或手机搜索esp32热点，然后连接，注意，某些手机需要手动选择 “在无互联网的情况下连接” 才能正确使用。
6. 打开浏览器，进入 http://192.168.8.1。
7. 设置参数然后点击重启。
8. 打开 http://你的服务器ip或域名:端口/，用80或443可以省略。
9. 你应该可以看到在线的设备了，点击设备名称右边的 "@" 可以访问内置Provider。


更多信息和示例请参考 /examples/。

### 参考头文件查看基础信息，配置在 /lib/config/config.h。

### LittleFS 额外组件自动选择功能还未在Windows环境测试过，但是它应该可以工作，目前在macOS 12.5工作正常。

### 更多教程和文档正在编写中。

### 如果你阅读了文档仍然不知道如何使用，我会录制一个教学视频，然后把视频连接放在这里。

# 感谢

感谢乐鑫的全体工作人员，让我们拥有了便宜好用的ESP32以及其他芯片。
感谢Node.js的全体开发者，让我们拥有了便捷好用的JavaScript运行时。
感谢VSCode和Platform IO的全体开发者，让我们拥有了好用的集成开发环境。

[Espressif](https://github.com/espressif)

[Node.js](https://github.com/nodejs)

[Visual Studio Code](https://github.com/microsoft/vscode)

[Platform IO](https://github.com/platformio)

[Brix](https://github.com/brix/crypto-js)

[Felix Biego](https://github.com/fbiego/ESP32Time)

# 协议

(GNU General Public License v3.0 License)

Copyright 2022, Vida Wang <github@vida.wang>

孩子是人类的未来，但是现在全世界仍然有许多孩子饱受饥饿，如果你是个善良的人、认可我的代码，请捐助联合国儿童基金会，谢谢。
https://unicef.cn
