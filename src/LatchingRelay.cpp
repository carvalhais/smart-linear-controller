#include "LatchingRelay.h"

LatchingRelay::LatchingRelay()
{
}

void LatchingRelay::begin(PCF8575 *io, int pinTx, int pinRx)
{
    _pinTx = pinTx;
    _pinRx = pinRx;
    _io = io;
    _started = true;
}

void LatchingRelay::toogle(bool state)
{
    if (!_started)
        return;
    _io->write(_pinTx, LOW);
    _io->write(_pinRx, LOW);
    uint8_t pin = state ? _pinTx : _pinRx;
    //DBG("LatchingRelay: Turning coil ON: PIN %d\n", pin);
    _io->write(pin, HIGH);
    _timer1 = millis() + _pulseWidth;
    _ongoing = true;
}

void LatchingRelay::loop()
{
    if (!_started)
        return;
    if (_ongoing && millis() > _timer1)
    {
        _ongoing = false;
        _io->write(_pinTx, LOW);
        _io->write(_pinRx, LOW);
        //DBG("LatchingRelay: Turning coils OFF\n");
    }
}
