#include "PowerSupplyScreen.h"

PowerSupplyScreen::PowerSupplyScreen()
{
}

void PowerSupplyScreen::drawSubHeader(const char *string, uint16_t x, uint16_t y, uint16_t w)
{
    _tft->drawFastHLine(x, y, w, TFT_DARKGREY);
    _tft->drawString(string, x + (w / 2), y + (_headerHeight / 2) + 2, 1);
    _tft->drawFastHLine(x, y + _headerHeight, w, TFT_DARKGREY);
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

    //_tft->fillRect(0, 0, _w, 80, TFT_NAVY);
    _tft->loadFont(_smallFont);
    _tft->drawString("Status: ", _x + (_meterWidth - 55), _y + _headerHeight + 5, 1);
    _tft->drawFastVLine(_x + _meterWidth, _y + _headerHeight, _headerHeight, TFT_DARKGREY);

    uint8_t y2 = _y + (_headerHeight * 2);

    _tft->drawFastHLine(_x, y2, _w, TFT_DARKGREY);
    _tft->drawFastVLine(_x + _meterWidth, y2, _headerHeight, TFT_DARKGREY);
    _tft->drawFastVLine(_x + _meterWidth * 2, y2, _headerHeight, TFT_DARKGREY);
    _tft->drawFastVLine(_x + _meterWidth * 3, y2, _headerHeight, TFT_DARKGREY);
    _tft->drawFastHLine(_x, y2 + _h - 1, w, TFT_DARKGREY);
    _tft->setTextDatum(MC_DATUM);

    drawSubHeader("DC OUT", _x, y2, _meterWidth);
    drawSubHeader("AC IN", _x + _meterWidth, y2, _meterWidth);
    drawSubHeader("TEMP IN", _x + (_meterWidth * 2), y2, _meterWidth);
    drawSubHeader("TEMP OUT", _x + (_meterWidth * 3), y2, _meterWidth);

    _tft->unloadFont();

    _spr = new TFT_eSprite(_tft);
    _spr->setColorDepth(8);
    _spr->createSprite(_w, _meterHeight);
    _spr->fillSprite(TFT_NAVY);
    _spr->drawFastVLine(_meterWidth, 0, _meterHeight, TFT_DARKGREY);
    _spr->drawFastVLine(_meterWidth * 2, 0, _meterHeight, TFT_DARKGREY);
    _spr->drawFastVLine(_meterWidth * 3, 0, _meterHeight, TFT_DARKGREY);
    _spr->drawFastHLine(0, _meterHeight - 1, _w, TFT_DARKGREY);

    _spr->setTextDatum(MC_DATUM);
    _spr->loadFont(_largeFont);

    _spr->pushSprite(_x, _y + (_headerHeight * 3) + 1);

    setMode("POWER OFF", TFT_WHITE);

    uint16_t barWidth = 350;
    _bargraph.begin(_x + ((_w - barWidth) / 2), _y + 123, barWidth, 60, tft, _microFont, _mediumFont, "OUTPUT CURRENT");
    _bargraph.drawScale();

    _update = true;
    loop();

    TouchRegion r1;
    r1.x = _x;
    r1.y = _y;
    r1.w = _w;
    r1.h = _h;
    r1.cmd = TouchCmd::PSU_BACK;

    _touchRegions = new TouchRegion[1];
    _touchRegions[0] = r1;
    _numTouchRegions = 1;
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

void PowerSupplyScreen::end()
{
    if (_spr != nullptr && _spr->created())
    {
        _spr->unloadFont();
        _spr->deleteSprite();
        delete _spr;
        _spr = nullptr;
    }
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

void PowerSupplyScreen::displayValue(uint8_t position, float value, uint8_t digits)
{
    uint16_t offset = (_meterWidth * position) + 1;
    _spr->fillRect(offset, 1, _meterWidth - 2, _meterHeight - 2, TFT_BLACK);

    if (value == -1)
    {
        _spr->drawString("-", offset + (_meterWidth / 2), 20);
    }
    else
    {
        _spr->drawFloat(value, digits, offset + (_meterWidth / 2), 20);
    }
}

void PowerSupplyScreen::loop()
{
    if (_update)
    {
        bool poff = _mode < PowerSupplyMode::NORMAL;

        if (_mode != _lastMode)
        {
            _lastMode = _mode;
            switch (_mode)
            {
            case PowerSupplyMode::NOT_STARTED:
                setMode("FAILED TO START", TFT_WHITE);
                break;
            case PowerSupplyMode::STARTED:
                setMode("STARTED", TFT_WHITE);
                break;
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
        }
        displayValue(0, poff ? -1 : _outputVoltage, 1);
        displayValue(1, poff ? -1 : _inputVoltage, 0);
        displayValue(2, poff ? -1 : _intakeTemperature, 0);
        displayValue(3, poff ? -1 : _outputTemperature, 0);
        _spr->pushSprite(_x, _y + (_headerHeight * 3) + 1);
        _bargraph.setValue(_current > 20 ? 20 : _current);
        // DBG("Power Supply: %.2f\n", _outputVoltage);
        _update = false;
    }
    _bargraph.loop();
}