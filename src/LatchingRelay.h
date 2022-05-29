#ifndef _LATCHING_RELAY_CLASS_
#define _LATCHING_RELAY_CLASS_

#include <Arduino.h>
#include <memory>
#include "Defines.h"

class LatchingRelay
{
public:
    LatchingRelay();
    void begin(int _pinLow, int _pinHigh);
    void toogle(bool state);
    void loop();

private:
    uint8_t _pinLow;
    uint8_t _pinHigh;
    volatile time_t _timer1;
    bool _ongoing = false;
    float _pulseWidth = 100;
};

#endif