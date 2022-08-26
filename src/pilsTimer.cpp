#include <pilsTimer.h>

PilsTimer::PilsTimer()
{
}

void PilsTimer::setup(const THandlerFunction &callback, unsigned long intervall)
{
    startTime = millis();
    this->callback = callback;
    this->intervall = intervall;
}

void PilsTimer::loop()
{
    if (millis() - startTime >= intervall)
    {
        callback();
        startTime = millis();
    }
}