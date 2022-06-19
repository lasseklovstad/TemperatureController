#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#ifndef APSSID
#define APSSID "Pils"
#define APPSK "pilserdigg"
#endif

/* Set these to your desired credentials. */
const char *ssidA = APSSID;
const char *passwordA = APPSK;

String getBatchUrl = "https://pilscontroller.herokuapp.com/api/batch";
const char *ssid = "Lauk";
const char *password = "SANDEFJORD";
bool ledOn = false;

unsigned long ledTimerStart;
unsigned long getBatchTimerStart;
unsigned long tryReconnectTimerStart;

HTTPClient http;
WebServer server(80);
Preferences preferences;

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
  default:
    return "Unknown error";
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

    // Save
    preferences.begin("pils-app", false);
    preferences.putString("ssid", ssidString);
    preferences.putString("password", passwordString);
    preferences.end();

    char ssid[100];
    char password[100];
    ssidString.toCharArray(ssid, ssidString.length() + 1);
    passwordString.toCharArray(password, passwordString.length() + 1);

    WiFi.begin(ssid, password);

    int retries = 0;

    while (WiFi.status() != WL_CONNECTED && retries <= 10)
    {
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

void tryReconnectWifi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    bool storeOpened = preferences.begin("pils-app", true);
    if (storeOpened == false)
    {
      preferences.end();
      return;
    }
    String storedSsid = preferences.getString("ssid");
    String storedpassword = preferences.getString("password");
    if (storedpassword != NULL && storedSsid != NULL)
    {
      char ssid[100];
      char password[100];
      storedSsid.toCharArray(ssid, storedSsid.length() + 1);
      storedpassword.toCharArray(password, storedpassword.length() + 1);
      preferences.end();
      WiFi.begin(ssid, password);
    }
    else
    {
      preferences.end();
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

  Serial.print("Configuring access point...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssidA, passwordA);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleForm);
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("Starting timers");
  initTimers();
  Serial.println("Init done");
}

void loop()
{
  server.handleClient();

  if (millis() - ledTimerStart >= 1000)
  {
    ledTimerStart = millis();
    turnLedOnOff();
  }

  if (millis() - getBatchTimerStart >= 10000)
  {
    getBatchTimerStart = millis();
    getBatch();
  }

  if (millis() - tryReconnectTimerStart >= 10000)
  {
    tryReconnectTimerStart = millis();
    tryReconnectWifi();
  }
}