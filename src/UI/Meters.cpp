#include "Meters.h"

Meters::Meters()
{
}

void Meters::begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, ReversePowerMode mode, const uint8_t smallFont[], const uint8_t mediumFont[])
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _tft = tft;

    _smallFont = (uint8_t *)smallFont;
    _mediumFont = (uint8_t *)mediumFont;
    _reversePowerMode = mode;

    _bandHeight = h / 2;
    _tft->drawFastHLine(x, y, w, TFT_DARKGREY);

    _paddingTop = (_bandHeight - _meterHeight) / 2;

    _meterInputPower.begin(_x + 3, _y + _paddingTop, (_w / 2) - 6, _meterHeight, tft, _smallFont, _mediumFont, "INPUT POWER (W)");
    _meterInputPower.setInitialScale(5);
    _meterInputPower.drawScale();

    _meterTemperature.begin(_x + (_w / 2) + 3, _y + _paddingTop, (_w / 2) - 6, _meterHeight, tft, _smallFont, _mediumFont, "TEMPERATURE (oC)");
    _meterTemperature.drawScale();
    _meterTemperature.setValue(_lastTemperature);

    _meterOutputPower.begin(_x + 3, _y + _bandHeight + _paddingTop, (_w / 2) - 6, _meterHeight, tft, _smallFont, _mediumFont, "OUTPUT POWER (W)");
    _meterOutputPower.setInitialScale(10);
    _meterOutputPower.drawScale();
    _meterOutputPower.showDecimal(false);
    _meterOutputPower.setValue(0);

    _meterOutputSwr.setReverseMode(_reversePowerMode);
    _meterOutputSwr.begin(_x + (_w / 2) + 3,
                          _y + _bandHeight + _paddingTop,
                          (_w / 2) - 6,
                          _meterHeight,
                          _tft,
                          _smallFont,
                          _mediumFont,
                          mode == MODE_SWR ? _headerSwr : _headerReversePower);
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
    // if (millis() > _timerWatts)
    // {
    //     DBG("OUTPUT: Forward: %.2f, Reverse: %.2f\n", forwardWatts, reverseWatts);
    //     _timerWatts = millis() + 1000;
    // }
    _meterOutputPower.setValue(forwardWatts);

    if (_reversePowerMode == ReversePowerMode::MODE_SWR)
    {
        if (forwardWatts > 0.0f && forwardWatts >= reverseWatts)
        {
            _meterOutputSwr.setValue(sqrt(reverseWatts / forwardWatts));
        }
        else
        {
            _meterOutputSwr.setValue(0);
        }
    }
    else
    {
        _meterOutputSwr.setValue(reverseWatts);
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

void Meters::setReverseMode(ReversePowerMode mode)
{
    _meterOutputSwr.setReverseMode(mode);
    _meterOutputSwr.setHeader(mode == MODE_SWR ? _headerSwr : _headerReversePower);
    _reversePowerMode = mode;
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
