#include "PowerSupplyScreen.h"

PowerSupplyScreen::PowerSupplyScreen()
{
}

void PowerSupplyScreen::begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TFT_eSPI *tft, const uint8_t microFont[], const uint8_t smallFont[], const uint8_t mediumFont[], const uint8_t largeFont[])
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _tft = tft;
    _microFont = (uint8_t *)microFont;
    _smallFont = (uint8_t *)smallFont;
    _mediumFont = (uint8_t *)mediumFont;
    _largeFont = (uint8_t *)largeFont;

    _tft->fillRect(_x, _y, _w, _h, TFT_BLACK);
    _meterWidth = _w / 4;
    _tft->setTextDatum(TL_DATUM);
    _tft->loadFont(_smallFont);
    _tft->drawCentreString("POWER SUPPLY MONITOR", x + (w / 2), y + 6, 1);
    _tft->drawFastHLine(_x, _y + _headerHeight, w, TFT_DARKGREY);
    _tft->loadFont(_smallFont);
    _tft->drawString("Status: ", _x + (_meterWidth - 55), _y + _headerHeight + 5, 1);
    _tft->drawFastVLine(_x + _meterWidth, _y + _headerHeight, _headerHeight, TFT_DARKGREY);
    _tft->setTextDatum(MC_DATUM);

    uint8_t y2 = _y + (_headerHeight * 2);

    _meterDCOut.begin("DC OUT (V)", _x, y2, _meterWidth, _headerHeight, _meterHeight, _tft, _smallFont, _largeFont);
    _meterACIn.begin("AC IN (V)", _x + _meterWidth, y2, _meterWidth, _headerHeight, _meterHeight, _tft, _smallFont, _largeFont);
    _meterTempIn.begin("TEMP IN (C)", _x + (_meterWidth * 2), y2, _meterWidth, _headerHeight, _meterHeight, _tft, _smallFont, _largeFont);
    _meterTempOut.begin("TEMP OUT (C)", _x + (_meterWidth * 3), y2, _meterWidth, _headerHeight, _meterHeight, _tft, _smallFont, _largeFont);

    uint16_t h2 = (_headerHeight + _meterHeight) * 2;

    _tft->drawFastVLine(_x + _meterWidth, y2, h2, TFT_DARKGREY);
    _tft->drawFastVLine(_x + _meterWidth * 2, y2, h2, TFT_DARKGREY);
    _tft->drawFastVLine(_x + _meterWidth * 3, y2, h2, TFT_DARKGREY);

    y2 += _headerHeight + _meterHeight;

    _meterCurrent.begin("CURRENT (A)", _x, y2, _meterWidth, _headerHeight, _meterHeight, _tft, _smallFont, _largeFont);
    _meterDCPower.begin("DC POWER (W)", _x + _meterWidth, y2, _meterWidth, _headerHeight, _meterHeight, _tft, _smallFont, _largeFont);
    _meterRFPower.begin("RF POWER (W)", _x + (_meterWidth * 2), y2, _meterWidth, _headerHeight, _meterHeight, _tft, _smallFont, _largeFont);
    _meterEfficiency.begin("EFFICIENCY (%)", _x + (_meterWidth * 3), y2, _meterWidth, _headerHeight, _meterHeight, _tft, _smallFont, _largeFont);

    setMode("POWER OFF", TFT_WHITE);

    uint16_t barWidth = 350;
    _bargraph.begin(_x + ((_w - barWidth) / 2), _h - 70, barWidth, 60, tft, _microFont, _mediumFont, "OUTPUT CURRENT");
    _bargraph.drawScale();

    _lastMode = PowerSupplyMode::NOT_STARTED;
    _update = true;
    loop();

    TouchRegion r1 = TouchRegion(_x, _y, _w, _h, TouchCmd::PSU_BACK);
    _numTouchRegions = 1;
    _touchRegions = new TouchRegion[_numTouchRegions]{r1};

    _init = true;
}

void PowerSupplyScreen::setStatus(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage)
{
    _mode = mode;
    _intakeTemperature = intTemp;
    _current = current;
    _outputVoltage = outVoltage;
    _inputVoltage = inputVoltage;
    _outputTemperature = outTemp;
    _update = true;
}

void PowerSupplyScreen::updateRFOutputPower(float power)
{
    _rfOutputPower = power;
    _update = true;
}

void PowerSupplyScreen::end()
{
    _meterDCOut.end();
    _bargraph.end();
}

void PowerSupplyScreen::setMode(const char *mode, uint32_t color)
{
    _tft->loadFont(_smallFont);
    _tft->setTextDatum(TL_DATUM);
    _tft->fillRect(_x + _meterWidth + 1, _y + _headerHeight + 1, _w - _meterWidth - 1, 19, 0x10A2); // almost black
    _tft->setTextColor(color, 0x10A2);
    _tft->drawString(mode, _x + _meterWidth + 5, _y + _headerHeight + 6, 1);
    _tft->unloadFont();
}

void PowerSupplyScreen::loop()
{
    if (_update)
    {
        bool poff = _mode < PowerSupplyMode::NORMAL;

        if (_mode != _lastMode)
        {
            switch (_mode)
            {
            case PowerSupplyMode::NOT_STARTED:
                setMode("FAILED TO START", TFT_WHITE);
                break;
            case PowerSupplyMode::STARTED:
            case PowerSupplyMode::POWEROFF:
                setMode("POWER OFF", TFT_WHITE);
                break;
            case PowerSupplyMode::NORMAL:
                setMode("NORMAL OPERATION", TFT_WHITE);
                break;
            case PowerSupplyMode::RAMPING:
                setMode("REACHING TARGET VOLTAGE", TFT_YELLOW);
                break;
            case PowerSupplyMode::WARNING:
                setMode("WARNING", TFT_YELLOW);
                break;
            case PowerSupplyMode::ALARM:
                setMode("ALARM", TFT_RED);
                break;
            default:
                break;
            }
            _lastMode = _mode;
        }

        _meterDCOut.setValue(poff ? -1 : _outputVoltage, 1);
        _meterACIn.setValue(poff ? -1 : _inputVoltage, 0);
        _meterTempIn.setValue(poff ? -1 : _intakeTemperature, 0);
        _meterTempOut.setValue(poff ? -1 : _outputTemperature, 0);

        float dcPower = _current * _outputVoltage;
        float efficiency = dcPower > 0 ? (_rfOutputPower / dcPower) * 100 : -1;
        if (efficiency > 100)
            efficiency = 0;

        _meterCurrent.setValue(poff ? -1 : _current, 1);
        _meterDCPower.setValue(poff ? -1 : dcPower, 0);
        _meterRFPower.setValue(poff ? -1 : _rfOutputPower, 0);
        _meterEfficiency.setValue(poff ? -1 : efficiency, 0);

        _bargraph.setValue(_current > 20 ? 20 : _current);
        _update = false;
    }
    _bargraph.loop();
}