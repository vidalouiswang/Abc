### English

This example show OTA update function.

Before you use this example, you need to do a little bit prepare:

* 1. Install Node.js 16 on your PC, offical download address:

```console
https://nodejs.org/
```

* 2. Use cmd or terminal open the path /server/

* 3. Run

```console
node iot.js
```

Or

```console
sudo node iot.js
```

To start a local server.

Don't forget let port pass in firewall.

* 4. Modify the IP address in "app.h" to your PC IP address(Local Area Network IP), line 131. 

* 5. Compile and upload firmware

* 6. Open your serial monitor, follow the guide in serial monitor

(If you didn't see anything output from monitor, press reset button on your ESP32 dev board.)


### 中文

这个例子展示OTA升级功能。

在你使用这个例子之前，你需要做一点点准备工作:

* 1. 在你的电脑上安装 Node.js, 官方下载地址:

```console
https://nodejs.org/
```

```console
http://nodejs.cn/
```

* 2. 使用 cmd 或终端打开路径 /server/

* 3. 运行

```console
node iot.js
```

或者

```console
sudo node iot.js
```

来开启一个本地服务器。

别忘了在防火墙中放行相应端口.

* 4. 修改文件 "app.h" 中的IP地址为你电脑所在的内网IP地址，131行

* 5. 编译上传固件

* 6. 打开你的串口监视器, 跟随串口监视器中的指引操作

(如果你没有在串口监视器中看到任何信息，按复位按钮。)

