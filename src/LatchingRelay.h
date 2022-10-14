#ifndef _LATCHING_RELAY_CLASS_
#define _LATCHING_RELAY_CLASS_

#include <Arduino.h>
#include <memory>
#include "Defines.h"
#include <PCF8575.h>

class LatchingRelay
{
public:
    LatchingRelay();
    void begin(PCF8575 *io, int pinTx, int pinRx);
    void toogle(bool state);
    void loop();

private:
    uint8_t _pinTx;
    uint8_t _pinRx;
    PCF8575 *_io;
    volatile time_t _timer1;
    bool _ongoing = false;
    float _pulseWidth = 100;
    bool _started = false;
};

#endif