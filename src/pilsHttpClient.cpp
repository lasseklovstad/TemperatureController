#include <pilsHttpClient.h>

PilsHttpClient::PilsHttpClient() {}

void PilsHttpClient::sendWarmTemperatureBatch(String warmBatchId, float temperatureC)
{
    if (isConnected())
    {
        Serial.println("Starting post batch");
        String url = postTemperatureUrl;
        url.replace(":batchId", warmBatchId);
        http.begin(url);
        http.setTimeout(3000);
        http.addHeader(SECRET_AUTH_HEADER_NAME, SECRET_AUTH_HEADER_VALUE);
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
}

float PilsHttpClient::getTemperature()
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
    return temperatureC1;
}

String PilsHttpClient::getWarmBatchId(boolean readOnly)
{
    bool storeOpened = preferences.begin("pils-app", readOnly);
    if (storeOpened == false)
    {
        return "";
    }
    return preferences.getString("warmBatchId", "");
}

void PilsHttpClient::setupTempSensors()
{
    if (isConnected())
    {
        float temperatureC = getTemperature();
        String warmBatchId = getWarmBatchId(true);
        if (warmBatchId != "")
        {
            sendWarmTemperatureBatch(warmBatchId, temperatureC);
        }
        preferences.end();
    }
}

void PilsHttpClient::getBatch()
{

    if (isConnected())
    {
        String warmBatchId = getWarmBatchId(false);
        if (warmBatchId == "")
        {
            http.begin(getBatchUrl);
            http.setTimeout(3000);
            http.addHeader(SECRET_AUTH_HEADER_NAME, SECRET_AUTH_HEADER_VALUE);
            int httpCode = http.POST("");
            Serial.println("Starting get batch");
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
                    preferences.putString("warmBatchId", payload.c_str());
                }
            }
            else
            {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
        }
        preferences.end();
    }
}

void PilsHttpClient::getIsActive()
{

    if (isConnected())
    {
        String warmBatchId = getWarmBatchId(false);
        if (warmBatchId != "")
        {
            String url = getActiveUrl;
            url.replace(":batchId", warmBatchId);
            http.begin(url);
            http.setTimeout(3000);
            http.addHeader(SECRET_AUTH_HEADER_NAME, SECRET_AUTH_HEADER_VALUE);
            int httpCode = http.GET();
            Serial.println("Starting get is batch active");
            if (httpCode > 0)
            {
                // HTTP header has been send and Server response header has been handled
                Serial.printf("[HTTP] GET... code: %d\n", httpCode);

                // file found at server
                if (httpCode == HTTP_CODE_OK)
                {
                    String payload = http.getString();
                    Serial.println(payload);
                    if (payload == "false")
                    {
                        Serial.println("Batch id inactive!");
                        preferences.putString("warmBatchId", "");
                    }
                }
            }
            else
            {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
        }
        preferences.end();
    }
}

void PilsHttpClient::postHasRestarted()
{

    if (hasRestarted && isConnected())
    {
        String warmBatchId = getWarmBatchId(false);
        if (warmBatchId != "")
        {
            String url = postHasRestartedUrl;
            url.replace(":batchId", warmBatchId);
            http.begin(url);
            http.setTimeout(3000);
            http.addHeader(SECRET_AUTH_HEADER_NAME, SECRET_AUTH_HEADER_VALUE);
            int httpCode = http.POST("");
            Serial.println("Starting post restarted");
            if (httpCode > 0)
            {
                // HTTP header has been send and Server response header has been handled
                Serial.printf("[HTTP] GET... code: %d\n", httpCode);

                // file found at server
                if (httpCode == HTTP_CODE_NO_CONTENT)
                {
                    Serial.println("Successfully sendt restart!");
                    hasRestarted = false;
                }
                else if (httpCode == HTTP_CODE_SERVICE_UNAVAILABLE)
                {
                    // Når id ikke finnes
                    Serial.println("Batch id finnes ikke!");
                    preferences.putString("warmBatchId", "");
                }
            }
            else
            {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
        }
    }
}