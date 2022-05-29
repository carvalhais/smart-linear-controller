#include "LatchingRelay.h"

LatchingRelay::LatchingRelay()
{
}

void LatchingRelay::begin(int pinLow, int pinHigh)
{
    _pinLow = pinLow;
    _pinHigh = pinHigh;
    pinMode(_pinLow, OUTPUT);
    pinMode(_pinHigh, OUTPUT);
}

void LatchingRelay::toogle(bool state)
{
    digitalWrite(_pinLow, 0);
    digitalWrite(_pinHigh, 0);

    DBG("LatchingRelay: Turning coil ON: PIN %s\n", state ? "HIGH" : "LOW");

    digitalWrite(state ? _pinHigh : _pinLow, 1);
    _timer1 = millis() + _pulseWidth;
    _ongoing = true;
}

void LatchingRelay::loop()
{
    if (_ongoing && millis() > _timer1)
    {
        _ongoing = false;
        digitalWrite(_pinLow, 0);
        digitalWrite(_pinHigh, 0);
        DBG("LatchingRelay: Turning coils OFF\n");
    }
}
