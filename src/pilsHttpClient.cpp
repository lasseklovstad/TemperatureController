#include <pilsHttpClient.h>

PilsHttpClient::PilsHttpClient()
{
    bool storeOpened = preferences.begin("pils-app", true);
    String storedControllerId = preferences.getString("controllerId", "");
    if (!storedControllerId.isEmpty())
    {
        controllerId = storedControllerId;
        Serial.println("Found controllerId: " + controllerId);
    }
    preferences.end();
}

void PilsHttpClient::postTemperature(float temperatureC, String type)
{
    if (isConnected() && !controllerId.isEmpty())
    {
        Serial.println("Starting post temp");
        String url = postTemperatureUrl;
        url.replace(":controllerId", controllerId);
        http.begin(url);
        http.setTimeout(3000);
        http.addHeader(SECRET_AUTH_HEADER_NAME, SECRET_AUTH_HEADER_VALUE);

        String body = "{ \"temp\": ";
        body += String(temperatureC);
        body += ", \"type\": \"";
        body += type;
        body += "\" }";

        Serial.println(body);

        int httpCode = http.POST(body);
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] POST... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_NO_CONTENT)
            {
                Serial.println("Temperature sent ok! " + type);
            }
        }
        else
        {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
}

void PilsHttpClient::getTemperature(float *temps)
{
    OneWire oneWire(oneWireBus);
    sensors.setOneWire(&oneWire);
    sensors.begin();
    int deviceCount = sensors.getDeviceCount();

    Serial.println("Temperature devices: " + deviceCount);

    sensors.requestTemperatures();
    float temperatureC1 = sensors.getTempCByIndex(0);
    float temperatureC2 = sensors.getTempCByIndex(1);
    Serial.println("Temp 1: " + String(temperatureC1) + "ºC");
    Serial.println("Temp 2: " + String(temperatureC2) + "ºC");
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
        Serial.println("Starting post microcontroller");
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] POST... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {
                String payload = http.getString();
                Serial.println(payload);
                // Save batch Id
                // Save
                preferences.begin("pils-app", false);
                preferences.putString("controllerId", payload.c_str());
                controllerId = payload;
                preferences.end();
            }
        }
        else
        {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
}

void PilsHttpClient::postHasRestarted()
{

    if (hasRestarted && !controllerId.isEmpty() && isConnected())
    {

        String url = postHasRestartedUrl;
        url.replace(":controllerId", controllerId);
        http.begin(url);
        http.setTimeout(3000);
        http.addHeader(SECRET_AUTH_HEADER_NAME, SECRET_AUTH_HEADER_VALUE);
        int httpCode = http.POST("");
        Serial.println("Starting post restarted");
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] POST... code: %d\n", httpCode);

            // file found at server
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
        }
        else
        {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
}