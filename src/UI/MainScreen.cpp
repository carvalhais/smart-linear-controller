#include "MainScreen.h"

MainScreen::MainScreen()
{
}

void MainScreen::begin(uint16_t x,
                       uint16_t y,
                       uint16_t w,
                       uint16_t h,
                       TFT_eSPI *tft,
                       const uint8_t microFont[],
                       const uint8_t smallFont[],
                       const uint8_t mediumFont[],
                       const uint8_t semiLarge[],
                       const uint8_t largeFont[])
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _tft = tft;
    _microFont = (uint8_t *)microFont;
    _smallFont = (uint8_t *)smallFont;
    _mediumFont = (uint8_t *)mediumFont;
    _semiLarge = (uint8_t *)semiLarge;
    _largeFont = (uint8_t *)largeFont;
    _bandHeight = 70;

    _tft->fillRect(x, y, w, h, TFT_BLACK);

    uint16_t height = SCREEN_HEIGHT > 240 ? 45 : 40;
    uint16_t metersHeight = height * 3;

    //_freq = new Frequency();
    // initialize the Frequency Widget
    _freq.begin(x, y, w, height, _tft, _smallFont, _mediumFont, _largeFont);

    //_meters = new Meters();
    // initialize the bargraphs (input and output meters)
    _meters.begin(x, y + height + 1, w, metersHeight, _tft, _microFont, _semiLarge);

    //_bottom = new Bottom();
    // "Digital" meters at the bottom
    _bottom.begin(x, y + height + metersHeight + 1, w, 40, _tft, _microFont, _mediumFont);

    _bottom.updateVolts(-1, TFT_DARKGREY);
    _bottom.updateAmperes(-1, TFT_DARKGREY);
    _bottom.updateGain(0);
    _bottom.updateFan(0);

    _btnBypass.begin(_tft, _mediumFont, _x + 42, _y + _h - 53, 80, 40, 3);
    _btnBypass.setText(TFT_WHITE, "BYPASS");

    setByPassState(false);

    TouchRegion r1;
    r1.x = _x;
    r1.y = _y + height + metersHeight;
    r1.w = _w / 2;
    r1.h = 40;
    r1.cmd = TouchCmd::MAIN_PSU;

    TouchRegion r2;
    r2.x = _btnBypass.xPos();
    r2.y = _btnBypass.yPos();
    r2.w = _btnBypass.width();
    r2.h = _btnBypass.height();
    r2.cmd = TouchCmd::MAIN_BYPASS;

    _numTouchRegions = 2;

    _touchRegions = new TouchRegion[_numTouchRegions];
    _touchRegions[0] = r1;
    _touchRegions[1] = r2;

    _update = true;
}

void MainScreen::updateOutputPower(float forwardWatts, float reverseWatts)
{
    _meters.updateOutputPower(forwardWatts, reverseWatts);
}

void MainScreen::updateGain(float gain)
{
    _bottom.updateGain(gain);
}

void MainScreen::updateInputPower(float forwardWatts)
{
    _meters.updateInputPower(forwardWatts);
}

void MainScreen::frequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState, char *band, bool txEnabled)
{
    _freq.frequencyChanged(frequency, modulation, filter, txState, band, txEnabled);
}

void MainScreen::updateTemperature(float temperature)
{
    _meters.updateTemperature(temperature);
}

void MainScreen::updateFanSpeed(uint8_t perc)
{
    _bottom.updateFan(perc);
}

void MainScreen::updatePowerSupply(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage)
{
    _mode = mode;
    _intakeTemperature = intTemp;
    _current = current;
    _outputVoltage = outVoltage;
    _inputVoltage = inputVoltage;
    _outputTemperature = outTemp;
    _update = true;
}

void MainScreen::setByPassState(bool state)
{
    if (state)
    {
        _btnBypass.setBackColors(0x3A8F, 0x1169, 0x6C13);
    }
    else
    {
        _btnBypass.setBackColors(0x2104, TFT_BLACK, TFT_WHITE);
    }
    _btnBypass.update();
    _freq.setTransmitEnabled(!state);
}

void MainScreen::loop()
{
    if (_update)
    {

        if (_mode < PowerSupplyMode::NORMAL)
        {
            _bottom.updateVolts(-1, TFT_DARKGREY);
            _bottom.updateAmperes(-1, TFT_DARKGREY);
        }
        else
        {
            uint32_t color = TFT_WHITE;
            switch (_mode)
            {
            case PowerSupplyMode::ALARM:
                color = TFT_RED;
                break;
            case PowerSupplyMode::WARNING:
            case PowerSupplyMode::RAMPING:
                color = TFT_YELLOW;
                break;
            default:
                break;
            }
            _bottom.updateAmperes(_current, color);
            _bottom.updateVolts(_outputVoltage, color);
        }
        _update = false;
    }
    _freq.loop();
    _meters.loop();
}

void MainScreen::end()
{
    _freq.end();
    _meters.end();
    _bottom.end();
    _btnBypass.end();
}
