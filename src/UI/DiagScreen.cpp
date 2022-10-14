#include "DiagScreen.h"

DiagScreen::DiagScreen()
{
}

void DiagScreen::begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t microFont[], const uint8_t smallFont[], const uint8_t mediumFont[], const uint8_t largeFont[])
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
    _colWidth = maximum(_colWidth, _w / 2);
}

void DiagScreen::update(Diag diag)
{
    _update = true;
    _diag = diag;
}

void DiagScreen::updatePowerSupply(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage)
{
    _mode = mode;
    switch (_mode)
    {
    case PowerSupplyMode::NOT_STARTED:
        _modeText = "FAILED";
        break;
    case PowerSupplyMode::STARTED:
        _modeText = "SEARCHING";
        break;
    case PowerSupplyMode::POWEROFF:
        _modeText = "POWER OFF";
        break;
    case PowerSupplyMode::NORMAL:
        _modeText = "NORMAL";
        break;
    case PowerSupplyMode::RAMPING:
        _modeText = "RAMP";
        break;
    case PowerSupplyMode::WARNING:
        _modeText = "WARNING";
        break;
    case PowerSupplyMode::ALARM:
        _modeText = "ALARM";
        break;
    default:
        _modeText = "UNKNOWN";
        break;
    }
    _update = true;
}

void DiagScreen::loop()
{
    if (_update)
    {
        _tft->fillRect(_x, _y, _w, _h, TFT_BLACK);
        _tft->setTextDatum(TL_DATUM);
        _tft->loadFont(_smallFont);
        _tft->drawCentreString("DIAGNOSTICS", _x + (_w / 2), _y + 5, 1);
        _tft->drawLine(_x, _y + 20, _x + _w, _y + 20, TFT_DARKGREY);
        _position = 30;

        printItem("I/O Expander (Main Board)", _diag.mainExpander);
        printItem("Main Board ADC", _diag.mainAdc);
        printItem("RF Sensor Board ADC", _diag.rfAdc);
        printItem("Temperature Sensor", _diag.temperature);
        if (_diag.temperature)
        {
            char temp[7];
            snprintf(temp, sizeof(temp), "%.1foC", _diag.temperatureCelsius);
            printDetail("Temperature", temp);
        }
        printItem("CAN BUS Driver (SJA1000)", _mode > PowerSupplyMode::NOT_STARTED);
        printDetail("PSU Status", _modeText);
        _tft->unloadFont();
        _update = false;
    }
}

void DiagScreen::printItem(String text, bool value)
{
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString(text, _x + _padLeft, _y + _position);
    _tft->textWidth(text);
    _tft->drawLine(_x + _padLeft + _tft->textWidth(text) + 2, _y + _position + 8, _x + _padLeft + _colWidth, _y + _position + 8, TFT_DARKGREY);
    _tft->drawString(":", _x + _padLeft + _colWidth + 2, _y + _position);
    _tft->setTextColor(value ? TFT_GREEN : TFT_RED, TFT_BLACK);
    _tft->drawString(value ? "OK" : "Fail", _x + _padLeft + _colWidth + _padLeft, _y + _position);
    _position += 20;
}

void DiagScreen::printDetail(String text, String value)
{
    _tft->setTextColor(TFT_GREY, TFT_BLACK);
    _tft->drawString(text, _x + _padLeft + 10, _y + _position);
    _tft->textWidth(text);
    _tft->drawLine(_x + _padLeft + _tft->textWidth(text) + 12, _y + _position + 8, _x + _padLeft + _colWidth, _y + _position + 8, TFT_GREY);
    _tft->drawString(":", _x + _padLeft + _colWidth + 2, _y + _position);
    _tft->setTextColor(TFT_GREY, TFT_BLACK);
    _tft->drawString(value, _x + _padLeft + _colWidth + _padLeft, _y + _position);
    _position += 20;
}

void DiagScreen::end()
{
}
