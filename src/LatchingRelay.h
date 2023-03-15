#ifndef _LATCHING_RELAY_CLASS_
#define _LATCHING_RELAY_CLASS_

#include <Arduino.h>
#include <memory>
#include <Defines.h>
#include <MCP23017.h>

class LatchingRelay
{
public:
    LatchingRelay();
    void begin(MCP23017 *io, int pinTx, int pinRx);
    void toogle(bool state);
    void loop();

private:
    uint8_t _pinTx;
    uint8_t _pinRx;
    MCP23017 *_io;
    volatile time_t _timer1;
    bool _ongoing = false;
    float _pulseWidth = 100;
    bool _started = false;
};

#endif