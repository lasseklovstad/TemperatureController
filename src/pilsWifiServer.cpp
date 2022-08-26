#include "pilsWifiServer.h"

PilsWifiServer::PilsWifiServer() : server(80)
{
}

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

void PilsWifiServer::handleForm()
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
        preferences.begin(APP_STORAGE_NAME, false);
        preferences.putString("ssid", ssidString.c_str());
        preferences.putString("password", passwordString.c_str());
        preferences.end();

        char ssid[100];
        char password[100];
        ssidString.toCharArray(ssid, ssidString.length() + 1);
        passwordString.toCharArray(password, passwordString.length() + 1);

        WiFi.begin(ssid, password);

        int retries = 0;

        PilsTimer retryTimer;
        bool connected = false;
        auto updateStatus = [&]()
        {
            connected = WiFi.status() == WL_CONNECTED;
            retries++;
        };
        retryTimer.setup(updateStatus, 500);

        while (!connected && retries <= 10)
        {
            retryTimer.loop();
        }

        server.send(200, "text/html", getPage());
    }
}

void PilsWifiServer::tryReconnectWifi()
{
    if (!isConnected())
    {
        bool storeOpened = preferences.begin(APP_STORAGE_NAME, true);
        String storedSsid = preferences.getString("ssid", "");
        String storedpassword = preferences.getString("password", "");

        if (storeOpened && storedpassword != "" && storedSsid != "")
        {
            Serial.println("Connecting to: " + storedSsid);
            char ssid[100];
            char password[100];
            storedSsid.toCharArray(ssid, storedSsid.length() + 1);
            storedpassword.toCharArray(password, storedpassword.length() + 1);
            WiFi.begin(ssid, password);
        }
        else
        {
            Serial.println("No ssid and password stored");
        }
        preferences.end();
    }
}

void PilsWifiServer::setup()
{
    Serial.print("Configuring access point...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    server.on("/", std::bind(&PilsWifiServer::handleForm, this));
    server.begin();
}

void PilsWifiServer::loop()
{
    server.handleClient();
}