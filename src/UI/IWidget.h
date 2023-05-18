#pragma once
#include <Arduino.h>

class IWidget
{
public:
    virtual ~IWidget(){};
    virtual uint16_t xPos() = 0;
    virtual uint16_t yPos() = 0;
    virtual uint16_t width() = 0;
    virtual uint16_t height() = 0;
};