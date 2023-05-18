#ifndef _CLASS_GRADIENT_BUTTON_
#define _CLASS_GRADIENT_BUTTON_

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Defines.h>
#include "IWidget.h"

class GradientButton : public IWidget
{
public:
    GradientButton();
    void begin(TFT_eSPI *tft, const uint8_t font[], uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t radius);
    void end();
    void setBackColors(uint16_t topColor, uint16_t bottomColor, uint16_t borderColor);
    void setDashed(bool dashed);
    void setText(uint16_t textColor, const char *text);
    void update();
    uint16_t xPos();
    uint16_t yPos();
    uint16_t width();
    uint16_t height();
    
private:
    TFT_eSprite *_spr;
    uint16_t _x;
    uint16_t _y;
    uint16_t _w;
    uint16_t _h;
    uint8_t _radius;

    uint16_t _topColor;
    uint16_t _bottomColor;
    uint16_t _borderColor;
    bool _dashed;

    uint16_t _textColor;
    const char *_text;
};

#endif