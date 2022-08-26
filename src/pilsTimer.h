#ifndef PILS_TIMER_H
#define PILS_TIMER_H
#include <Arduino.h>
typedef std::function<void(void)> THandlerFunction;
class PilsTimer
{
private:
    unsigned long intervall;
    unsigned long startTime;
    THandlerFunction callback;

public:
    PilsTimer();
    void setup(const THandlerFunction &callback, unsigned long intervall);
    void loop();
};

#endif