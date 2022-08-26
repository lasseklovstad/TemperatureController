#ifndef PILS_WIFI_SERVER_H
#define PILS_WIFI_SERVER_H
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <pilsUtils.h>
#include <secrets.h>
#include <pilsTimer.h>

class PilsWifiServer
{
private:
    WebServer server;
    Preferences preferences;
    const char *ssid = SECRET_WIFI_SSID;
    const char *password = SECRET_WIFI_PASSWORD;

    void handleForm();

public:
    PilsWifiServer();
    void tryReconnectWifi();
    void setup();
    void loop();
};

#endif