#include "Button.h"

Button::Button()
{
}

void Button::begin(int pin, ButtonCb callback)
{
    _button.attach(pin, INPUT_PULLUP);
    _button.interval(5);
    _button.setPressedState(LOW);
    _buttonPressCallback = callback;
}

void Button::loop()
{
    bool changed = _button.update();

    if (_button.isPressed() && _button.currentDuration() >= LONG_PRESS_THRESOLD && !_longPressFired)
    {
        _longPressFired = true;
        if (_buttonPressCallback)
            _buttonPressCallback(_button.getPin(), true);
    }
    if (changed && _button.released())
    {
        if (_longPressFired)
        {
            _longPressFired = false;
        }
        else
        {
            if (_buttonPressCallback)
                _buttonPressCallback(_button.getPin(), false);
        }
    }
}
