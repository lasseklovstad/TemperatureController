#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Ticker.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

String getBatchUrl = "https://pilscontroller.herokuapp.com/api/batch";
const char *ssid = "Lauk";
const char *password = "SANDEFJORD";
bool ledOn = false;
Ticker ticker;
Ticker ledTicker;

void initWiFi()
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  int k = 0;
  while (WiFi.status() != WL_CONNECTED && k < 10)
  {
    Serial.print('.');
    delay(500);
    k++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    ESP.restart();
  }

  Serial.println(WiFi.localIP());
}

void getBatch()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    Serial.println("Starting get batch");
    http.begin(getBatchUrl);
    http.setTimeout(3000);
    Serial.println("http begin");
    http.addHeader("xxxauth", "halla");
    Serial.println("Add header");
    int httpCode = http.GET();
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

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  WiFi.disconnect(true);
  initWiFi();
  delay(1000);

  Serial.println("Starting timers");
  ticker.attach(10, getBatch);
  ledTicker.attach(1, turnLedOnOff);
  Serial.println("Init done");
}

void loop()
{
}