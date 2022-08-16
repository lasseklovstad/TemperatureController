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
    String getBatchUrl = baseUrl + "batch/warm";
    String postTemperatureUrl = baseUrl + ":batchId/temperature";
    String getActiveUrl = baseUrl + ":batchId/active";
    String postHasRestartedUrl = baseUrl + ":batchId/restarted";
    const int oneWireBus = 4;
    DallasTemperature sensors;
    Preferences preferences;
    float getTemperature();
    String getWarmBatchId(boolean readOnly);
    boolean hasRestarted = true;

public:
    PilsHttpClient();
    void sendWarmTemperatureBatch(String warmBatchId, float temperatureC);
    void setupTempSensors();
    void getBatch();
    void getIsActive();
    void postHasRestarted();
};

#endif