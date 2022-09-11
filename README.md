# Abc

[中文](https://github.com/vidalouiswang/Abc/blob/main/README_CN.md)

### A basic ESP32 library for beginners

## At present, the ESP32 firmware built with this library has been running stably for several months and can be used officially.

### Note: Clone the code from main branch.

## Brief

This library work with Espressif offical framework esp32-arduino 2.0.3.
Only had tested on esp32 dev module.

This library is for beginners who want to write their own code, but don't have much foundation. This library can be your starting point for learning esp32, you can use not only all the functions of esp32-arduino, but also esp-idf function, you can learn how to develop ESP32 from simple to further.

### Features

* Built-in convenient OTA update without extra code.
* Rollback firmware when new frimware error at boot state automatically(esp32-arduino library closed this function from esp-idf, this library implemented this feature without modify SDK that downloaded by Platform IO).
* Easy method to store or transfer integer, string, binary data without other format(like json).
* Built-in convenient RAM database could store key-value typed data, based on LittleFS(esp-idf has this function but it don't has wear-leveling, according to offical document).
* Stable and fast Websocket implement.
* One line code to get SHA1 and SHA256 using ESP32 hardware acceleration with variant types of input data.
* One line code to encrypt or decrypt data with AES-256-CBC.
* More features are in development...

These features especially designed for beginners, easy to use and have strong scalability. For experts, please ignore.

# How to use

IDE requirement: Visual studio code with Platform IO installed.

First you need to have a VPS, node.js 16 required.
Or you could use your PC or NAS as a server.

### Step 1: Clone

```console
git clone https://github.com/vidalouiswang/Abc.git
```

### Step 2: Setup Server

1. Use SSH connect to your VPS or NAS

2. Install package manager , suppose your OS is Ubuntu

```console
sudo apt-get install npm
```

3. Install n module
```console
sudo npm install -g n
```

4. Install Node.js
```console
sudo n stable
```

5. Install pm2
```console
sudo npm install -g pm2
```

Now the version of node.js is lastest stable version, you could use the following command to check

```console
node -v
```

If shows "v16.x" means Node.js installed correctly, if it show old version, like "v8.x", you should input command

```console
hash -r
```

6. If you clone the code into your personal computer, you should upload
/server/
    - index.html
    - ab.js
    - hash.js
    - create.js
    - iot.js

These five files to your server

```console
cd ~/
```
```console
mkdir server
```
```console
cd server
```

Then upload five files by any method you like to the folder "server"

This step could be omit if you clone the code directly to your server, do this:

```console
cd Abc/server
```

7. Install components
```console
sudo npm install ws
```

8. Start server
```console
sudo pm2 start iot.js
```

Now the server configuration is finished, the default port is 12345 , you could modify it if you like,

in file "globalConfig.json".

Or, modify it at the bottom in file "iot.js".

Remember restart server if you modified the port.

```console
sudo pm2 restart iot.js
```

Don't forget add rule to let new port could pass through in firewall.

### Step 3: Local Use

1. Use Platform IO open root folder.
2. Locate to /src/app/ . 
3. Edit app.h and app.cpp as Arduino way.
4. Compile and upload(better erase flash first).
5. Use PC or phone connect to esp32 access point.
6. Open browser，locate to http://192.168.8.1.
7. Set arguments by tips and click reboot.
8. Locate to http://your_domain_or_ip_of_your_server:port/.
9. Login in with user name and password set in step 7.
10. Now you could see your device online, click "@" could access built-in providers.

More information and examples refer to /examples/ , full comments in it.

### Check header files for basic information, configs locate to /lib/config/config.h

### The LittleFS extra component auto-selection feature has not been tested on Windows, but it should work and currently works fine on macOS 12.5.

### More tutorials and documentation are in the works.

### If you still don't know how to use, I will make a video and upload to YouTube, the link will be put right here.

### Code comments status(full and detailed comments):

* Local:

    * Header file:

        * Almost all header files
        * /examples/*.h

    * Source File:

        * /lib/arraybuffer/arraybuffer.cpp
        * /src/main/main.cpp
        * /examples/*.cpp

* Server:

    * /server/iot.js
    * /server/create.js

# Thanks to

Thanks to the entire staff of Espressif, we have a cheap and easy-to-use ESP32 and other chips.
Thanks to all the developers of Node.js, we have a convenient and easy-to-use JavaScript runtime.
Thanks to all the developers of VSCode and Platform IO, we have an easy-to-use integrated development environment.

[Espressif](https://github.com/espressif)

[Node.js](https://github.com/nodejs)

[Visual Studio Code](https://github.com/microsoft/vscode)

[Platform IO](https://github.com/platformio)

[ws](https://github.com/websockets/ws)

[Brix](https://github.com/brix/crypto-js)

[Felix Biego](https://github.com/fbiego/ESP32Time)

# License

(GNU General Public License v3.0 License)

Copyright 2022, Vida Wang <github@vida.wang>

Children are the future of mankind, but there are still many children who are enduring hunger all over the world at this moment. If you are a good person and you like this lib, please donate to UNICEF.
https://unicef.org
