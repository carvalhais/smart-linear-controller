#ifndef _BUTTON_CLASS_
#define _BUTTON_CLASS_

#include <Arduino.h>
#include <Bounce2.h>
#include <memory>
#include "Defines.h"

#define LONG_PRESS_THRESOLD 300

typedef std::function<void(int pin, bool longPress)> ButtonCb;

class Button
{
public:
    Button();
    void begin(int pin, ButtonCb callback);
    void loop();

private:
    ButtonCb _buttonPressCallback;
    bool _longPressFired = false;
    Bounce2::Button _button;
};

#endif