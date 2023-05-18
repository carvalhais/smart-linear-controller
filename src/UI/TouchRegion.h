#pragma once

#include <Arduino.h>
#include <Types.h>
#include <TFT_eSPI.h>
#include <Defines.h>
#include "IWidget.h"

class TouchRegion
{
public:
    TouchRegion();
    TouchRegion(IWidget &widget, TouchCmd cmd);
    TouchRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TouchCmd cmd);
    void drawOutline(TFT_eSPI *tft, uint32_t color);
    uint16_t xPos();
    uint16_t yPos();
    uint16_t width();
    uint16_t height();
    TouchCmd command();

private:
    uint16_t _x;
    uint16_t _y;
    uint16_t _w;
    uint16_t _h;
    TouchCmd _cmd;
    uint8_t _border = 5;
    void setCoords(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    const char *_touchCmdText[5] = {"TOUCH_NONE", "MAIN_PSU", "MAIN_BYPASS", "MAIN_STANDBY", "PSU_BACK"};
};
