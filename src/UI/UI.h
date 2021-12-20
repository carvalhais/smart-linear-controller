#ifndef _UI_CLASS_
#define _UI_CLASS_

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Defines.h"

class UI
{
public:
    UI();
    void begin(TFT_eSPI *tft, const uint8_t smallFont[]);
    void end();
    void drawHeader();
    void clearScreen();

protected:
    uint8_t *_smallFont;
    uint8_t *_mediumFont;
    TFT_eSPI *_tft;
    std::unique_ptr<TFT_eSprite> _spr;
};

#endif