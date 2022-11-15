This component enable the system update firmware over the air.
More than 10000 times OTA update had been tested, it is very stable.
If any error detected on your ESP32 while using OTA update, please check your remote server first.
There are full comments in header file, check ota.h for more information.

* If got low connection quality between your device and AP it connected, or between your server and your network, OTA update may stuck during it process, don't reboot your device or upload again, leave it, it will update successfully.

这个组件可以让系统空中升级固件。
已经经过了至少10000次的升级测试，它很稳定。
如果你在OTA升级过程中遇到了问题，请先检查你的服务器。
头文件中有完整的注释，更多信息请参阅ota.h。

* 如果你的设备与它连接的AP连接质量不高或你的服务器和你的网络连接质量不高，在OTA升级的时候有时候会卡住，但是不用重启设备或重新上传固件，不用管它，它自己会更新成功的。