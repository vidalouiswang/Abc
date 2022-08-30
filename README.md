# Abc

### A basic esp32 library for beginners(Beta)

[中文](https://github.com/vidalouiswang/Abc/blob/main/README_CN.md)

This library work with Espressif offical framework esp32-arduino 2.0.3.
Only had tested on esp32 dev module.

This library is for those men who wants to code themselves but they don't have too much code skills or they are beginners, then you could start from here to learn how to program esp32.

# How to use

First you need to have a VPS, node.js 16 required.
Or you could use your PC or NAS as a server.

### Clone

```console
git clone https://github.com/vidalouiswang/Abc.git
```

### Upload

    /server/
        - pro/
            - index.html
        - ab.js
        - hash.js
        - create.js
        - iot.js

five files to your server.
(These 5 files should be placed in the same directory, the tree only tells you which 5 files should be upload)

### Then run

```console
npm install ws
```

```console
npm install pm2
```

```console
pm2 start iot.js
```

### Local

1. Use Platform IO open root folder.
2. Locate to /src/app/ . 
3. Edit app.h and app.cpp.
4. Compile and upload firmware(better erase flash first).
5. Use PC or phone connect to esp32 access point, attention, some phone you need to select "keep connection without internet".
6. Open browser，locate to http://192.168.8.1.
7. Set arguments and click reboot.
8. Locate to http://your_domain_or_ip_of_your_server:port/, 80 or 443 could be omit.
9. Now you could see your device online, click "@" could access built-in providers.

More information and examples refer to /examples/.

### Check header files for basic information, configs locate to /lib/config/config.h

### The LittleFS extra component auto-selection feature has not been tested on Windows, but it should work and currently works fine on macOS 12.5.

### More tutorials and documentation are in the works

# Thanks to

Thanks to the entire staff of Espressif, we have a cheap and easy-to-use ESP32 and other chips.
Thanks to all the developers of Node.js, we have a convenient and easy-to-use JavaScript runtime.
Thanks to all the developers of VSCode and Platform IO, we have an easy-to-use integrated development environment.

[Espressif](https://github.com/espressif)

[Node.js](https://github.com/nodejs)

[Visual Studio Code](https://github.com/microsoft/vscode)

[Platform IO](https://github.com/platformio)

[Brix](https://github.com/brix/crypto-js)

[Felix Biego](https://github.com/fbiego/ESP32Time)

# License

(GNU General Public License v3.0 License)

Copyright 2022, Vida Wang <github@vida.wang>

Children are the future of mankind, but there are still many children who are enduring hunger all over the world at this moment. If you are a good person and you like this lib, please donate to UNICEF.
https://unicef.org
