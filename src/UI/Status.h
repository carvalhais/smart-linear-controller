#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Types.h>
#include "IWidget.h"

class Status : public IWidget
{
public:
    Status();
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t largeFont[]);
    void end();
    uint16_t xPos();
    uint16_t yPos();
    uint16_t width();
    uint16_t height();

private:
    uint16_t _x;
    uint16_t _y;
    uint16_t _w;
    uint16_t _h;
};
