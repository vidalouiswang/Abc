2023/02/07

English:

1. Fix bug that AP won't start on new device after firmware uploaded. 

中文: 

1. 修复了bug: 新设备在烧录固件后AP不会开启。

2023/02/05

English:

1. Add auto sync function, which means you could modify source file in /app/appName, it will be copied to /src/app automatically. 
2. Fix bug of web client and add cr, lf, crlf or none to serial mode. The cr, lf, crlf will be appended to content automatically.
3. Add timeout check process to auto OTA update, the auto OTA update process will be restart when error detected.
4. Fix bug of tools "replaceHtml.js".
5. Update version of main system to 42.1.0. Refer to /lib/config/config.h.
6. Add quick interface to global manager to set RX buffer size of serial0.
7. Remove a repeat function of global manager "inline uint16_t getRemoteServerOfflineDetectedTimes()". Use "inline bool serverOnline(uint8_t serverStatus = 0)" to instead.
8. Add feature to let user manually force system won't start AP in any situations. Be careful to use this function, or the system will NOT enable AP when sta connection failed or other status, this will reduce heat from SoC which help to other sensor that sensitive to temperature.
9. The time of firmware compiled will be copied to /lib/globalmanager/globalmanager.h automatically, it will shows in web client to let user can get time info.
10. Fix bug that AP won't close automatically.
11. Fix bug that transfer data from hardware serial to web client. It will use stack to hold data when size of data less than 128 bytes, or it will use heap to hold data. The maximum size of data defined as 81916 bytes, which is large enough.

Conclusion: Currently the whold system running with no error, it is stable.

中文:

1. 增加了源代码自动同步辅助功能，在/app/app名称下修改源代码会自动同步到/src/app下。
2. 修复了前端的bug，给serial数据转发增加了在末尾自动添加特殊字符的功能，可以选择cr、lf、crlf或无特殊字符。
3. 给自动OTA升级功能增加了超时检测功能，自动OTA在发生错误超时 时会自动重新对设备发起升级请求。
4. 修复了工具"replaceHtml.js"中的路径引用错误。
5. 更新系统版本号到 "42.1.0"，在头文件/lib/config/config.h中。
6. 增加了快速设置硬件串口RX缓冲区大小的功能。
7. 移除了一个globalmanager冗余的函数"inline uint16_t getRemoteServerOfflineDetectedTimes()"，现在使用"inline bool serverOnline(uint8_t serverStatus = 0)"代替。
8. 增加了功能可以让用户手动执行强制禁止开启AP，使用此功能系统在**任何状态下**都**不会**开启AP，这会减少SoC的热量，以减少对周围距离较近的对温度敏感的传感器的影响。
9. 固件的编译时间会被自动填充到头文件/lib/globalmanager/globalmanager.h中，这个时间可以在前端看到，方便确定固件的具体编译时间。
10. 修复了AP不会自动关闭的bug。
11. 修复了串口数据转发的bug，当转发的数据大小小于128字节时，将使用栈空间存放数据，否则将使用堆空间存储数据，最大数据量被定义为81916字节，这足够大了。

结论: 当前系统运行无任何错误，非常稳定。
