#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Types.h"
#include "Defines.h"

class SimpleMeter
{
public:
    SimpleMeter();
    void begin(const char *header, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t h2, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t largeFont[]);
    void end();
    void setValue(float value, uint8_t digits);

private:
    TFT_eSPI *_tft;
    TFT_eSprite *_spr;
    const char *_header;
    uint8_t *_smallFont;
    uint8_t *_largeFont;
    uint16_t _x;
    uint16_t _y;
    uint16_t _w;
    uint16_t _h;
    uint16_t _h2;
    float _lastValue = -255;
    uint8_t _padRight = 7;
};