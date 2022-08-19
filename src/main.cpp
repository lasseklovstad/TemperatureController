#include <Arduino.h>
#include <pilsWifiServer.h>
#include <pilsHttpClient.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

bool ledOn = false;

unsigned long ledTimerStart;
unsigned long getBatchTimerStart;
unsigned long tryReconnectTimerStart;

PilsWifiServer pilsWifiServer;
PilsHttpClient pilsHttpClient;

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
    pilsHttpClient.postHasRestarted();
    pilsHttpClient.postMicroController();
  }

  if (millis() - tryReconnectTimerStart >= 10000)
  {
    tryReconnectTimerStart = millis();
    pilsWifiServer.tryReconnectWifi();
    pilsHttpClient.readTemperatureSensorsAndPost();
  }
}