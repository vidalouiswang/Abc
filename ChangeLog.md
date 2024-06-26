2024/05/01

English:
1. Fix bug of build tool.
2. Now support ESP32-C3.

中文:

1. 修复了构建工具的bug。
2. 现在支持ESP32-C3。


2023/09/03

English:
1. Bug fixed: path indexing error for S3.

中文:
1. 错误已修复: S3的路径索引错误。

2023/09/03

English:
1. An auotomatic function added to the codes, it could help you to convert old file format to new.
Check tail of "config.h".
(Better backup database and files before you do this)
2. ESP32-S3 support added, the default ESP32-S3 env added to the "platformio.ini".
3. A bug fixed. This bug could make app loop won't run.

中文:
1. 向代码中添加了一个函数可以自动转换旧版本的数据，查看"config.h"的尾部。(最好在转换前先备份数据库和文件)
2. 现已支持ESP32-S3，默认的ESP32-S3环境参数已添加到 "platformio.ini"。
3. 修复了一个错误，此错误可能会导致esp32的app loop不会运行。


2023/04/02

English:

1. Unify mark for transfer and type of class Element.
2. Change function name from "webSerial" to "sendMessageToClient" of class GlobalManager. 
3. Change argument type of "sendMessageToClient" to Element&, which is more convenient to use. New usage: "global->sendMessageToClient(100); global->sendMessageToClient(3.14); global->sendMessageToClient("Hello world!"); ...".
4. Change bytes format of Element from custom to little endian, attenation, this is NOT compatible with former version, MyDB will be interacted.
5. Because of bytes format changed, there will be a function added after, it could update former version.

中文:

1. 统一了用于传输的标志和类 Element 的类型。
2. 类 GlobalManager 的成员函数 "webSerial" 的名称修改为 "sendMessageToClient"。
3. "sendMessageToClient" 参数类型修改为 Element& ， 这可以让使用更加方便。
4. 更改了类 Element 存储数据的字节序，由自定义字节序修改为小段存储，注意，这个修改不兼容以前的版本，内置的数据库组件将受到影响。
5. 由于更改了字节序，之后会添加一个函数用于更新已存储的数据到新的版本。

2023/03/30

English:

1. Change data format to standard little endian.
2. ETYPE_STRING will copy last '\0'.
3. Move part of processes of create buffer and decode buffer to class Element.
4. Data processing of JS had been synchronized with cpp.

中文:

1. 修改数据格式为标准小端存储。
2. 字符串类型现在会拷贝末尾的 '\0'。
3. 编解码 Elements 的一部分处理过程移动到类 Element。
4. JS的数据处理过程现已与C++同步。

2023/03/28

English:

1. Bug fixed:
    * Non-copy mode of class Element for buffer.

2023/03/26

English:

1. Remove redundant function "getU8ALen" and "getBufferLength" of class Element, use "getRawBufferLength" to instead.
2. Replace data type from uint64_t to uint32_t of static functions in class ArrayBuffer.

中文: 

1. 移除了冗余的成员函数 "getU8ALen" 和 "getBufferLength"，使用 "getRawBufferLength" 来代替。
2. 类 ArrayBuffer 中的静态函数的数据类型由 uint64_t 改为 uint32_t。

2023/03/24

English:

1. Reduce RAM cost from 24 bytes to 16 bytes of class Element by reorder member sequence.
2. Remove member "bufferLength", use union data member to hold length of buffer.

中文:

1. 通过重新对成员排序，使类 Element 的栈内存消耗由24字节缩小为16字节。
2. 移除了数据成员 bufferLength，改为使用联合体成员实现同样的功能。

2023/03/24

English:

1. Bug fixed:
    * Remove "\r\n" in /scripts/replaceHtml.js to avoid too many empty lines in globalmmanager.h following compile time. 
    * Fix SHA logical error in class Element.

2. Add convenient functions of Base64, SHA256 and AES to class Element.
3. Add more unit test to /test/test.cpp.


中文: 

1. 错误修复: 
    * 移除了 /scripts/replaceHtml.js 中多余的 "\r\n" 来避免文件 globalmanager.h 中编译时间后多余的空行。
    * 修复了 类 Element 中 SHA 的逻辑错误。

2. 给 类 Element 增加了方便的Base64、SHA256、AES 相关函数。
3. 添加了更多的单元测试到 /test/test.cpp。

2023/03/22

English:

1. Add all types supported to class Element, u8, i8, u16, i16, u32, i32, u64, i64, float, double, string, buffer. Some name of enum were changed. Reference to arraybuffer.hpp.
2. Add convenient method to support Android WebView to get custom ID, Android Studio example project will release later.


中文:

1. 给类 Element 添加了所有数据类型的支持，单字节、双字节、四字节的有符号和无符号整数，单精度浮点、双精度浮点，字符串和二进制数组。 修改了某些枚举的名称。
2. 添加了方便的方法来给安卓WebView快速获取自定义ID，Android Studio 示例工程后续会上传。

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
