#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Ticker.h>
#include <WiFiClient.h>
#include <WebServer.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#ifndef APSSID
#define APSSID "ESPap"
#define APPSK "thereisnospoon"
#endif

/* Set these to your desired credentials. */
const char *ssidA = APSSID;
const char *passwordA = APPSK;

WebServer server(80);

String getBatchUrl = "https://pilscontroller.herokuapp.com/api/batch";
const char *ssid = "Lauk";
const char *password = "SANDEFJORD";
bool ledOn = false;
Ticker ticker;
Ticker ledTicker;

String getWifiStatusText()
{
  wl_status_t status = WiFi.status();
  switch (status)
  {
  case WL_CONNECTED:
    return "Connected";
  case WL_CONNECT_FAILED:
    return "Failed";
  case WL_CONNECTION_LOST:
    return "Connection lost";
  case WL_NO_SSID_AVAIL:
    return "No ssid available";
  case WL_IDLE_STATUS:
    return "Idle";
  case WL_DISCONNECTED:
    return "Disconnected";
  case WL_SCAN_COMPLETED:
    return "Scan complete";
  case WL_NO_SHIELD:
    return "No shield";
  }
}

String createHtmlOption(String label, String value)
{
  return "<option value=\"" + value + "\">" + label + "</option>";
}

String getPage()
{

  int scanResult = WiFi.scanNetworks();
  String menuItems = "";
  String status = getWifiStatusText();

  if (scanResult == 0)
  {
    Serial.println(F("No networks found"));
  }
  else if (scanResult > 0)
  {
    Serial.printf(PSTR("%d networks found:\n"), scanResult);

    // Print unsorted scan results
    for (int8_t i = 0; i < scanResult; i++)
    {
      String ssid = WiFi.SSID(i);
      menuItems += createHtmlOption(ssid, ssid);
    }
  }
  else
  {
    Serial.printf(PSTR("WiFi scan error %d"), scanResult);
  }

  return "<html>\
  <head>\
    <meta charset=\"utf-8\" />\
    <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" />\
    <title>Koble til internett</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      form {display: flex; flex-direction: column;}\
    </style>\
  </head>\
  <body>\
    <h1>Koble til wifi</h1>\
    <h2>Status: " +
         status + "</h2>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/\">\
      <label for=\"password\">Skriv inn passord</label>\
      <input type=\"text\" id=\"password\" name=\"password\" />\
      <label for=\"internett\">Velg internett</label>\
      <select name=\"internett\" id=\"internett\">\
        " +
         menuItems + "\
      </select>\
      <br/>\
      <br/>\
      <button type=\"submit\">Send</button>\
    </form>\
  </body>\
</html>";
}

void handleForm()
{
  if (server.method() == HTTP_GET)
  {
    server.send(200, "text/html", getPage());
  }
  else if (server.method() == HTTP_POST)
  {

    String ssidString = server.arg(1);
    String passwordString = server.arg(0);
    char ssid[100];
    char password[100];
    ssidString.toCharArray(ssid, ssidString.length() + 1);
    passwordString.toCharArray(password, passwordString.length() + 1);

    WiFi.begin(ssid, password);
    // Broadcast to arduino
    Serial.println();
    Serial.print("ssid=" + String(ssid));
    Serial.println();
    Serial.print("password=" + String(password));
    Serial.println();
    Serial.print("Connecting");

    int retries = 0;

    while (WiFi.status() != WL_CONNECTED && retries <= 10)
    { // Wait for the Wi-Fi to connect
      delay(500);
      Serial.print(".");
      retries++;
    }
    Serial.println();
    Serial.print("Status" + getWifiStatusText());
    Serial.println();
    Serial.print("IsConnected" + WiFi.isConnected());
    Serial.println();
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    server.send(200, "text/html", getPage());
  }
}

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

  Serial.print("Configuring access point...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssidA, passwordA);
  digitalWrite(BUILTIN_LED, HIGH);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleForm);
  server.begin();
  Serial.println("HTTP server started");

  delay(1000);

  Serial.println("Starting timers");
  ticker.attach(10, getBatch);
  ledTicker.attach(1, turnLedOnOff);
  Serial.println("Init done");
}

void loop()
{
  server.handleClient();
}