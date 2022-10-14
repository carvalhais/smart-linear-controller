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

    _meterWidth = _w / 4;

    _tft->setTextDatum(MC_DATUM);
    _tft->loadFont(_smallFont);
    drawSubHeader("VOLTS", 0, 0, _meterWidth);
    drawSubHeader("AMPS", _meterWidth, 0, _meterWidth);
    drawSubHeader("GAIN", _meterWidth * 2, 0, _meterWidth);
    drawSubHeader("FAN", _meterWidth * 3, 0, _meterWidth);
    _tft->unloadFont();

    //_tft->loadFont(_mediumFont);
    _tft->drawFastVLine(_x + _meterWidth, _y, _headerHeight, TFT_DARKGREY);
    _tft->drawFastVLine(_x + _meterWidth * 2, _y, _headerHeight, TFT_DARKGREY);
    _tft->drawFastVLine(_x + _meterWidth * 3, _y, _headerHeight, TFT_DARKGREY);
    _tft->drawFastHLine(_x, _y + _h - 1, w, TFT_DARKGREY);

    _spr = new TFT_eSprite(tft);
    uint8_t sprHeight = h - _headerHeight - 2;
    //_spr->setColorDepth(8);
    _spr->createSprite(w, sprHeight);
    _spr->setTextDatum(MC_DATUM);
    _spr->loadFont(_mediumFont);
    _spr->fillSprite(_bgColor);
    _spr->drawFastVLine(_meterWidth, 0, sprHeight, TFT_DARKGREY);
    _spr->drawFastVLine(_meterWidth * 2, 0, sprHeight, TFT_DARKGREY);
    _spr->drawFastVLine(_meterWidth * 3, 0, sprHeight, TFT_DARKGREY);
    _spr->setTextColor(TFT_WHITE, _bgColor);
    _spr->pushSprite(_x, _y + _headerHeight + 1);
}

void Bottom::drawSubHeader(const char *string, uint16_t x, uint16_t y, uint16_t w)
{
    _tft->drawFastHLine(_x + x, _y + y, w, TFT_DARKGREY);
    _tft->drawString(string, _x + x + (w / 2), _y + (_headerHeight / 2), 1);
    _tft->drawFastHLine(_x + x, _y + y + _headerHeight, w, TFT_DARKGREY);
}

void Bottom::end()
{
    if (_spr != nullptr && _spr->created())
    {
        _spr->deleteSprite();
        delete _spr;
        _spr = nullptr;
    }
}

void Bottom::setValue(const char *text, uint16_t x, uint32_t color)
{
    uint16_t x0 = x + 1;
    uint16_t y0 = 1;
    uint8_t w0 = _meterWidth - 1;
    uint8_t h0 = _h - _headerHeight - 2;
    _spr->fillRect(x0, y0, w0, h0, _bgColor);
    _spr->setTextColor(color, _bgColor);
    _spr->drawString(text, x0 + (_meterWidth / 2), y0 + (h0 / 2) + 1, 1);
    _spr->pushSprite(_x, _y + _headerHeight + 1);
}

void Bottom::updateVolts(float volts, uint32_t color)
{
    if (volts != -1)
    {
        char str[6] = {0};
        sprintf(str, "%1.1fV", volts);
        setValue(str, 0, color);
    }
    else
    {
        setValue("-", 0, color);
    }
}

void Bottom::updateAmperes(float amps, uint32_t color)
{
    if (amps != -1)
    {
        char str[7] = {0};
        sprintf(str, "%1.1fA", amps);
        setValue(str, _meterWidth, color);
    }
    else
    {
        setValue("-", _meterWidth, color);
    }
}

void Bottom::updateGain(float gain)
{
    char str[7] = {0};
    sprintf(str, "%1.1fdB", gain);
    setValue(str, _meterWidth * 2, TFT_WHITE);
}

void Bottom::updateFan(uint8_t perc)
{
    char temp[8];
    if (perc == 0)
    {
        sprintf(temp, "OFF");
    }
    else
    {
        snprintf(temp, sizeof(temp), "%d%%", perc);
    }
    setValue(temp, _meterWidth * 3, TFT_WHITE);
}