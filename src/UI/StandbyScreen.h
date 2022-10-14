#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Types.h"
#include "Defines.h"
#include "ScreenBase.h"

class StandbyScreen : public ScreenBase
{
public:
    StandbyScreen();
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TFT_eSPI *tft, const uint8_t smallFont[]);
    void end();

private:
    TFT_eSPI *_tft;
    uint8_t *_smallFont;
    uint16_t _x;
    uint16_t _y;
    uint16_t _w;
    uint16_t _h;
};