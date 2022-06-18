#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "Lauk";
const char *password = "SANDEFJORD";



void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void getBatch(){
  HTTPClient http;
  Serial.println("Starting get batch");
  http.begin("https://pilscontroller.herokuapp.com/api/batch");
  http.addHeader("xxxauth", "halla");
  int httpCode = http.GET();
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
}

void setup()
{
  Serial.begin(115200);
  initWiFi();
}

void loop()
{
  getBatch();
  delay(10000);
}