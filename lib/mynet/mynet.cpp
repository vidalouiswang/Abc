#include "mynet.h"

bool MyNet::startAP(const char *ssid, IPAddress *apIP, const char *pwd)
{
    this->apIP = apIP;
    if (!ssid)
    {
        return false;
    }
    if (!strlen(ssid))
    {
        return false;
    }

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.enableSTA(true);
    WiFi.enableAP(true);

    WiFi.onEvent(
        [](arduino_event_t *event) -> void
        {
            WiFi.softAPConfig(*(myNet.apIP), *(myNet.apIP), IPAddress(255, 255, 255, 0));
        },
        ARDUINO_EVENT_WIFI_AP_START);
    if (pwd)
    {
        if (strlen(pwd))
        {
            WiFi.softAP(ssid, pwd);
        }
        else
        {
            WiFi.softAP(ssid);
        }
    }
    else
    {
        WiFi.softAP(ssid);
    }
    return true;
}
MyNet myNet;