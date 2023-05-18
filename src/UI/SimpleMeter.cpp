#include "SimpleMeter.h"

SimpleMeter::SimpleMeter()
{
}
void SimpleMeter::begin(const char *header, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t h2, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t largeFont[])
{
    _tft = tft;
    _header = header;
    _smallFont = (uint8_t *)smallFont;
    _largeFont = (uint8_t *)largeFont;
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _h2 = h2;
    _tft->loadFont(_smallFont);
    _tft->setTextDatum(MR_DATUM);
    _tft->drawString(header, x + w - _padRight, y + (h / 2) + 2);
    _tft->unloadFont();
    _tft->drawFastHLine(x, y, w, TFT_DARKGREY);
    _tft->drawFastHLine(x, y + h, w, TFT_DARKGREY);
    _tft->drawFastHLine(x, y + h + h2, w, TFT_DARKGREY);
    _lastValue = -255;
}

void SimpleMeter::setValue(float value, uint8_t digits)
{
    if (value != _lastValue)
    {
        // in order to avoid flickering, we pad left the resulting string
        // this way the graphic library understand it need to fill the background
        _tft->loadFont(_largeFont);
        _tft->setTextColor(TFT_WHITE, TFT_BLACK, true);
        _tft->setTextDatum(MR_DATUM);
        char buffer[10];
        if (value == -1)
        {
            snprintf(buffer, 9, "%8s", "-");
        }
        else
        {
            char tmp[10];
            snprintf(tmp, 9, "%.*f", digits, value);
            snprintf(buffer, 9, "%8s", tmp);
        }
        _tft->drawString(buffer, _x + _w - _padRight, _y + _h + (_h2 / 2) + 3);
        _tft->unloadFont();
        _lastValue = value;
    }
}

void SimpleMeter::end()
{
}
