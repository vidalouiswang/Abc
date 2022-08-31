#include "app.h"
#include "../src/globalmanager/globalmanager.h"

void App::setup()
{
    Serial.println("1. Open your browser 打开你的浏览器\n");

    Serial.println("2. Redirect to \"http://127.0.0.1:12345/\" or \"http://localhost:12345/\"");
    Serial.println("转到\"http://127.0.0.1:12345/\" 或者 \"http://localhost:12345/\"");

    Serial.println("3. Use \"abc12345678\" as user name and password");
    Serial.println("使用 \"abc12345678\" 作为用户名和密码\n");

    Serial.println("4. Login 登录\n");

    Serial.println("5. You can see esp32 is online 你可以看见esp32在线\n");

    Serial.println("6. Click \"@\" 点击 \"@\"\n");

    Serial.println("7. Click \"Upload firmware\" 点击 \"上传固件\"\n");

    Serial.println("8. Choose a firmware 选择一个固件\n");

    Serial.println("9. Click OK 点击确定\n");

    Serial.println("10. Click \"OTA Update\" 点击 \"更新固件\"\n");

    Serial.println("Then you will see esp32 is updating firmware, it will reboot after finished");
    Serial.println("然后你就可以看到esp32正在更新固件了, 更新完成后会自动重启");
}

void App::loop()
{
}

/**
 * @brief App instance defined here
 * 
 * 类App实例在此实例化
 * 
 */
App *app = new App();