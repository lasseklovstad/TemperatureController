#include <Arduino.h>
#include <pilsWifiServer.h>
#include <pilsHttpClient.h>
#include <pilsTimer.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

bool ledOn = false;

PilsWifiServer pilsWifiServer;
PilsHttpClient pilsHttpClient;
PilsTimer ledTimer;
PilsTimer reconnectTimer;
PilsTimer microControllerTimer;

auto toggleLedd = [&]()
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
};

auto handleMicroController = [&]()
{
  pilsHttpClient.postHasRestarted();
  pilsHttpClient.postMicroController();
  pilsHttpClient.readTemperatureSensorsAndPost();
};

auto handleReconnect = [&]()
{
  pilsWifiServer.tryReconnectWifi();
  // Serial.println(ESP.getFreeHeap());
};

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  pilsWifiServer.setup();
  pilsHttpClient.setup();

  ledTimer.setup(toggleLedd, 1000);
  reconnectTimer.setup(handleReconnect, 10000);
  microControllerTimer.setup(handleMicroController, 10000);

  Serial.println("Init done");
}

void loop()
{
  pilsWifiServer.loop();
  ledTimer.loop();
  microControllerTimer.loop();
  reconnectTimer.loop();
}