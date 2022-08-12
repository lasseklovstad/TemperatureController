#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <pilsWifiServer.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#ifndef APSSID
#define APSSID "Pils"
#define APPSK "pilserdigg"
#endif

const int oneWireBus = 4;
boolean hasSensorsBegun = false;
DallasTemperature sensors;

String getBatchUrl = "https://pils.gataersamla.no/api/microcontroller/batch/warm";
String postTemperatureUrl = "https://pils.gataersamla.no/api/microcontroller/:batchId/temperature";
const char *ssid = "Lauk";
const char *password = "SANDEFJORD";
bool ledOn = false;

unsigned long ledTimerStart;
unsigned long getBatchTimerStart;
unsigned long tryReconnectTimerStart;

HTTPClient http;
Preferences preferences;
PilsWifiServer pilsWifiServer;

void sendWarmTemperatureBatch(String warmBatchId, float temperatureC)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Starting post batch");
    String url = postTemperatureUrl;
    url.replace(":batchId", warmBatchId);
    http.begin(url);
    http.setTimeout(3000);
    http.addHeader("xxxauth", "halla");
    int httpCode = http.POST(String(temperatureC));
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_NO_CONTENT)
      {
        Serial.println("Temperature sent ok!");
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.println("Wifi not connected");
  }
}

void setupTempSensors()
{
  if (WiFi.status() == WL_CONNECTED && hasSensorsBegun == false)
  {
    OneWire oneWire(oneWireBus);
    sensors.setOneWire(&oneWire);
    sensors.begin();
    Serial.println("Sensors begun");
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");

    // Check if there exists another batch in memory
    bool storeOpened = preferences.begin("pils-app", true);
    if (storeOpened == false)
    {
      preferences.end();
      return;
    }
    String warmBatchId = preferences.getString("warmBatchId");
    if (warmBatchId != NULL)
    {
      sendWarmTemperatureBatch(warmBatchId, temperatureC);
    }
  }
}

void getBatch()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Starting get batch");
    http.begin(getBatchUrl);
    http.setTimeout(3000);
    Serial.println("http begin");
    http.addHeader("xxxauth", "halla");
    Serial.println("Add header");
    int httpCode = http.POST("");
    Serial.println("Start get");
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();
        Serial.println(payload);
        // Save batch Id
        // Save
        preferences.begin("pils-app", false);
        preferences.putString("warmBatchId", payload.c_str());
        preferences.end();
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.println("Wifi not connected");
  }
}

void turnLedOnOff()
{
  if (ledOn == true)
  {
    digitalWrite(BUILTIN_LED, LOW);
    ledOn = false;
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH);
    ledOn = true;
  }
}

void initTimers()
{
  ledTimerStart = millis();
  getBatchTimerStart = millis();
  tryReconnectTimerStart = millis();
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  pilsWifiServer.setup();

  Serial.println("Starting timers");
  initTimers();
  Serial.println("Init done");
}

void loop()
{
  pilsWifiServer.loop();

  if (millis() - ledTimerStart >= 1000)
  {
    ledTimerStart = millis();
    turnLedOnOff();
  }

  if (millis() - getBatchTimerStart >= 10000)
  {
    getBatchTimerStart = millis();
    Serial.println("Running batch");
    // Check if there exists another batch in memory
    bool storeOpened = preferences.begin("pils-app", true);
    if (storeOpened == false)
    {
      preferences.end();
    }
    else
    {
      String warmBatchId = preferences.getString("warmBatchId");
      preferences.end();
      if (warmBatchId == NULL)
      {
        getBatch();
      }
    }
  }

  if (millis() - tryReconnectTimerStart >= 10000)
  {
    tryReconnectTimerStart = millis();
    Serial.println("Running reconnect");
    pilsWifiServer.tryReconnectWifi();
    setupTempSensors();
  }
}