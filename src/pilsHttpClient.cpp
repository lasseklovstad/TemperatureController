#include <pilsHttpClient.h>

void startPost(HTTPClient &http, String &url, const char *body, boolean json, const std::function<void(int)> &callback)
{
    http.begin(url);
    http.setTimeout(3000);
    http.addHeader(SECRET_AUTH_HEADER_NAME, SECRET_AUTH_HEADER_VALUE);
    if (json)
    {
        http.addHeader("Content-Type", "application/json");
    }
    int httpCode = http.POST(body);
    if (httpCode > 0)
    {
        callback(httpCode);
        if (httpCode >= 400)
        {
            Serial.println(String("Post error: ") + httpCode + " " + url);
        }
    }
    http.end();
}

PilsHttpClient::PilsHttpClient()
{
}

void PilsHttpClient::setup()
{

    bool storeOpened = preferences.begin(APP_STORAGE_NAME, true);
    if (!storeOpened)
    {
        Serial.println("Store failed to open");
        preferences.end();
    }
    String storedControllerId = preferences.getString("controllerId", "");
    if (!storedControllerId.isEmpty())
    {
        controllerId = storedControllerId;
        Serial.println("Found controllerId: " + controllerId);
    }
    else
    {
        Serial.println("Not Found controllerId: " + storedControllerId);
    }

    preferences.end();
}

void PilsHttpClient::postTemperature(const float temperatureC, const char *type)
{
    if (isConnected() && !controllerId.isEmpty())
    {
        String url = postTemperatureUrl;
        url.replace(":controllerId", controllerId);

        String body = "{ \"temp\": ";
        body += String(temperatureC);
        body += ", \"type\": \"";
        body += type;
        body += "\" }";

        auto onSuccess = [&](int httpCode)
        {
            if (httpCode == HTTP_CODE_NO_CONTENT)
            {
                // Ok!
            }
            else if (httpCode == HTTP_CODE_NOT_FOUND)
            {
                controllerId.clear();
            }
        };

        startPost(http, url, body.c_str(), true, onSuccess);
    }
}

void PilsHttpClient::getTemperature(float *temps)
{
    OneWire oneWire(oneWireBus);
    sensors.setOneWire(&oneWire);
    sensors.begin();
    int deviceCount = sensors.getDeviceCount();

    sensors.requestTemperatures();
    float temperatureC1 = sensors.getTempCByIndex(0);
    float temperatureC2 = sensors.getTempCByIndex(1);

    temps[0] = temperatureC1;
    temps[1] = temperatureC2;
}

void PilsHttpClient::readTemperatureSensorsAndPost()
{
    if (isConnected() && controllerId != NULL)
    {
        float *temps = new float[2];
        getTemperature(temps);
        postTemperature(temps[0], "WARM");
        postTemperature(temps[1], "COLD");
        delete[] temps;
    }
}

void PilsHttpClient::postMicroController()
{

    if (isConnected() && controllerId.isEmpty())
    {
        http.begin(postMicroControllerUrl);
        http.setTimeout(3000);
        http.addHeader(SECRET_AUTH_HEADER_NAME, SECRET_AUTH_HEADER_VALUE);
        int httpCode = http.POST("Pils Controller 1");
        auto onSuccess = [&](int httpCode)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                String payload = http.getString();
                Serial.println(payload);
                preferences.begin(APP_STORAGE_NAME, false);
                preferences.putString("controllerId", payload.c_str());
                controllerId = payload;
                preferences.end();
            }
        };
        startPost(http, postMicroControllerUrl, "Pils Controller 1", false, onSuccess);
    }
}

void PilsHttpClient::postHasRestarted()
{

    if (hasRestarted && !controllerId.isEmpty() && isConnected())
    {

        String url = postHasRestartedUrl;
        url.replace(":controllerId", controllerId);
        auto onSuccess = [&](int httpCode)
        {
            if (httpCode == HTTP_CODE_NO_CONTENT)
            {
                Serial.println("Successfully sendt restart!");
                hasRestarted = false;
            }
            else if (httpCode == HTTP_CODE_SERVICE_UNAVAILABLE)
            {
                // Når id ikke finnes
                Serial.println("Controller id finnes ikke!");
                controllerId.clear();
            }
        };
        startPost(http, url, "", false, onSuccess);
    }
}