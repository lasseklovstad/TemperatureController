#ifndef PILS_WIFI_SERVER_H
#define PILS_WIFI_SERVER_H
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <pilsUtils.h>

#ifndef APSSID
#define APSSID "Pils"
#define APPSK "pilserdigg"
#endif

class PilsWifiServer
{
private:
    WebServer server;
    Preferences preferences;
    const char *ssid = APSSID;
    const char *password = APPSK;

    void handleForm();

public:
    PilsWifiServer();
    void tryReconnectWifi();
    void setup();
    void loop();
};

#endif