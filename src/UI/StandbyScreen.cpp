#include "StandbyScreen.h"

StandbyScreen::StandbyScreen()
{
}

void StandbyScreen::begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TFT_eSPI *tft, const uint8_t smallFont[])
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _tft = tft;
    _smallFont = (uint8_t *)smallFont;
    _tft->loadFont(_smallFont);
    _tft->fillRect(x, y, w, h, TFT_BLACK);
    _tft->setTextDatum(MC_DATUM);
    _tft->loadFont(_smallFont);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString("WAITING FOR BLUETOOTH...", w / 2, y + (h / 2), 1);
    _tft->unloadFont();
}

void StandbyScreen::end()
{
}
