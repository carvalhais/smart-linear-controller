#include "Meters.h"

Meters::Meters()
{
}

void Meters::drawSubHeader(const char *string, uint16_t x, uint16_t y, uint16_t w)
{
    _tft->drawFastHLine(x, y, w, TFT_DARKGREY);
    _tft->drawCentreString(string, x + (w / 2), y + 3, 1);
    _tft->drawFastHLine(x, y + 14, w, TFT_DARKGREY);
}

void Meters::begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[])
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _tft = tft;

    _smallFont = (uint8_t *)smallFont;
    _mediumFont = (uint8_t *)mediumFont;

    _tft->loadFont(_smallFont);
    // freq to input meters separator
    drawSubHeader("INPUT POWER / SWR", x, y, w);
    // input meters to output meters separator
    drawSubHeader("OUTPUT POWER / SWR", x, y + 55, w);
    _tft->unloadFont();

    _meterInputPower.begin(_x + 3, _y + 20, 182, 30, tft, _smallFont, _mediumFont);
    _meterInputPower.drawScale();

    _meterInputSwr.begin(_x + 188, _y + 20, 127, 30, tft, _smallFont, _mediumFont);
    _meterInputSwr.drawScale();

    _meterOutputPower.begin(_x + 3, _y + 74, 182, 40, tft, _smallFont, _mediumFont);
    _meterOutputPower.drawScale();

    _meterOutputSwr.begin(_x + 188, _y + 74, 127, 40, tft, _smallFont, _mediumFont);
    _meterOutputSwr.drawScale();
}

void Meters::end()
{
    _meterInputPower.end();
    _meterInputSwr.end();
    _meterOutputPower.end();
    _meterOutputSwr.end();
}

void Meters::updateInputSwr(float forwardMv, float reverseMv)
{
    float fwdWatts = pow(forwardMv, 2) * 10;
    _meterInputPower.setValue(fwdWatts);

    //DBG("INPUT: Fwd: %1.3f, Rev: %1.3f, Watts: %1.2f, Ratio: %1.3f\n", forwardMv, reverseMv, fwdWatts, (forwardMv > 0 ? reverseMv / forwardMv : 0));

    if (forwardMv > 0 && forwardMv >= reverseMv)
    {
        _meterInputSwr.setValue(reverseMv / forwardMv);
    }
    else
    {
        _meterInputSwr.setValue(0);
    }
}

void Meters::updateOutputSwr(float forwardMv, float reverseMv)
{
    float fwdWatts = pow(forwardMv, 2) * 10;
    _meterOutputPower.setValue(fwdWatts);

    //DBG("OUTPUT: Fwd: %1.3f, Rev: %1.3f, Watts: %1.2f, Ratio: %1.3f\n", forwardMv, reverseMv, fwdWatts, (forwardMv > 0 ? reverseMv / forwardMv : 0));

    if (forwardMv > 0 && forwardMv >= reverseMv)
    {
        _meterOutputSwr.setValue(reverseMv / forwardMv);
    }
    else
    {
        _meterOutputSwr.setValue(0);
    }
}

void Meters::loop()
{
    if (_next <= millis())
    {
        _next = millis() + 100;
        _meterInputPower.loop();
        _meterInputSwr.loop();
        _meterOutputPower.loop();
        _meterOutputSwr.loop();
    }
}
