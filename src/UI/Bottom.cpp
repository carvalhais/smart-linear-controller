#include "Bottom.h"

Bottom::Bottom()
{
}

void Bottom::begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[])
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _tft = tft;

    _smallFont = (uint8_t *)smallFont;
    _mediumFont = (uint8_t *)mediumFont;

    _spr = std::unique_ptr<TFT_eSprite>(new TFT_eSprite(tft));
    _spr->createSprite(_w, _h);
    _spr->loadFont(_smallFont);
    drawSubHeader("VOLTS", 0, 0, 80);
    drawSubHeader("AMPS", 80, 0, 80);
    drawSubHeader("TEMP", 160, 0, 80);
    drawSubHeader("ANT", 240, 0, 80);
    _spr->unloadFont();

    _spr->drawFastVLine(80, 0, _h, TFT_DARKGREY);
    _spr->drawFastVLine(160, 0, _h, TFT_DARKGREY);
    _spr->drawFastVLine(240, 0, _h, TFT_DARKGREY);
    _spr->drawFastHLine(0, _h - 1, w, TFT_DARKGREY);

    _spr->pushSprite(_x, _y);
}

void Bottom::drawSubHeader(const char *string, uint16_t x, uint16_t y, uint16_t w)
{
    _spr->drawFastHLine(x, y, w, TFT_DARKGREY);
    _spr->drawCentreString(string, x + (w / 2), y + 3, 1);
    _spr->drawFastHLine(x, y + 14, w, TFT_DARKGREY);
}

void Bottom::end()
{
    if (_spr->created())
    {
        _spr->deleteSprite();
    }
}

void Bottom::setValue(char *text, uint16_t x, uint16_t y)
{
    _spr->setTextDatum(MC_DATUM);
    _spr->loadFont(_mediumFont);
    _spr->setTextColor(TFT_WHITE, TFT_BLACK);
    _spr->drawString(text, x, y, 1);
    _spr->unloadFont();
    _spr->pushSprite(_x, _y);
}

void Bottom::updateVolts(float volts)
{
    char str[6] = {0};
    sprintf(str, "%1.1fV", volts);
    setValue(str, 40, 27);
}

void Bottom::updateAmperes(float amps)
{
    char str[7] = {0};
    sprintf(str, "%1.1fA", amps);
    setValue(str, 120, 27);
}

void Bottom::updateTemperature(float temp)
{
    char str[7] = {0};
    sprintf(str, "%1.1fC", temp);
    setValue(str, 200, 27);
}

void Bottom::updateAntenna(char *ant)
{
    setValue(ant, 280, 27);
}