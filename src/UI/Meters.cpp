#include "Meters.h"

Meters::Meters()
{
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

    uint8_t bandHeight = h / 2;
    uint8_t meterHeight = 54;

    _tft->drawFastHLine(x, y, w, TFT_DARKGREY);

    uint16_t t1 = w * .5f;
    uint16_t t2 = w * .5f;

    uint8_t paddingTop = (bandHeight - meterHeight) / 2;

    _meterInputPower.begin(_x + 3, _y + paddingTop, t1 - 6, meterHeight, tft, _smallFont, _mediumFont, "INPUT POWER (W)");
    _meterInputPower.setInitialScale(5);
    _meterInputPower.drawScale();

    _meterTemperature.begin(_x + t1 + 3, _y + paddingTop, t2 - 6, meterHeight, tft, _smallFont, _mediumFont, "TEMPERATURE (oC)");
    _meterTemperature.drawScale();
    if (_lastTemperature > 0)
    {
        _meterTemperature.setValue(_lastTemperature);
    }
    _meterOutputPower.begin(_x + 3, _y + bandHeight + paddingTop, t1 - 6, meterHeight, tft, _smallFont, _mediumFont, "OUTPUT POWER (W)");
    _meterOutputPower.setInitialScale(10);
    _meterOutputPower.drawScale();
    _meterOutputPower.showDecimal(false);
    _meterOutputPower.setValue(0);

    _meterOutputSwr.begin(_x + t1 + 3, _y + bandHeight + paddingTop, t2 - 6, meterHeight, tft, _smallFont, _mediumFont, "OUTPUT SWR");
    _meterOutputSwr.drawScale();

    _updated = true;
}

void Meters::end()
{
    _meterInputPower.end();
    _meterTemperature.end();
    _meterOutputPower.end();
    _meterOutputSwr.end();
}

void Meters::updateOutputPower(float forwardWatts, float reverseWatts)
{
    // DBG("OUTPUT: Watts: %1.2f, Ratio: %1.3f\n", forwardWatts, (forwardWatts > 0 ? reverseWatts / forwardWatts : 0));

    _meterOutputPower.setValue(forwardWatts);
    if (forwardWatts > 0.0f && forwardWatts >= reverseWatts)
    {
        _meterOutputSwr.setValue(reverseWatts / forwardWatts);
    }
    else
    {
        _meterOutputSwr.setValue(0);
    }
    _updated = true;
}

void Meters::updateInputPower(float forwardWatts)
{
    _meterInputPower.setValue(forwardWatts);
    // DBG("INPUT: Watts: %1.2f\n", forwardWatts);
    _updated = true;
}

void Meters::updateTemperature(float temperature)
{
    _meterTemperature.setValue(temperature);
    _lastTemperature = temperature;
    _updated = true;
}

void Meters::loop()
{
    if (_updated || millis() > _next)
    {
        _next = millis() + 100;
        _meterInputPower.loop();
        _meterTemperature.loop();
        _meterOutputPower.loop();
        _meterOutputSwr.loop();
    }
}
