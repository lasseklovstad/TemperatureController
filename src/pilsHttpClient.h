#ifndef PILS_HTTP_CLIENT_H
#define PILS_HTTP_CLIENT_H
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>
#include <pilsUtils.h>
#include <secrets.h>

class PilsHttpClient
{
private:
    HTTPClient http;
    String baseUrl = "https://pils.gataersamla.no/api/microcontroller/";
    String postMicroControllerUrl = baseUrl;
    String postTemperatureUrl = baseUrl + ":controllerId/temperature";
    String postHasRestartedUrl = baseUrl + ":controllerId/restarted";
    String controllerId;
    const int oneWireBus = 4;
    DallasTemperature sensors;
    Preferences preferences;
    void getTemperature(float *temps);
    boolean hasRestarted = true;

public:
    PilsHttpClient();
    void postTemperature(const float temperatureC, const char *type);
    void readTemperatureSensorsAndPost();
    void postMicroController();
    void postHasRestarted();
    void setup();
};

#endif