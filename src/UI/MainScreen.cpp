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

    uint16_t y2 = y + height + 1;
    _meters.begin(x, y2, w, metersHeight, _tft, _reverseMode, _microFont, _semiLarge);

    TouchRegion r1 = TouchRegion(x + (w / 2), y2 + (height * 2) - 10, w / 2, height, TouchCmd::MAIN_SWR_MODE);

    y2 += metersHeight;
    _bottom.begin(x, y2, w, height, _tft, _microFont, _mediumFont);
    TouchRegion r2 = TouchRegion(x, y2, w / 2, height, TouchCmd::MAIN_PSU);
    uint16_t quarter = w / 4;
    //_status.begin(quarter * 2, )
    _btnBypass.begin(_tft, _mediumFont, _x + 20, _y + _h - 53, 80, 40, 3);
    _btnBypass.setText(TFT_WHITE, "BYPASS");
    setByPassState(_lastByPassState);
    TouchRegion r3 = TouchRegion(_btnBypass, TouchCmd::MAIN_BYPASS);
    _numTouchRegions = 3;
    _touchRegions = new TouchRegion[_numTouchRegions]{r1, r2, r3};
    // if (true)
    // {
    //     r1.drawOutline(_tft, TFT_RED);
    //     r2.drawOutline(_tft, TFT_YELLOW);
    //     r3.drawOutline(_tft, TFT_PINK);
    // }
    _update = true;
    _initialized = true;
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
    _lastByPassState = state;
}

void MainScreen::setReverseMode(ReversePowerMode mode)
{
    if (mode != _reverseMode)
    {
        _reverseMode = mode;
        if (_initialized)
            _meters.setReverseMode(mode);
    }
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
    _initialized = false;
    _freq.end();
    _meters.end();
    _bottom.end();
    _btnBypass.end();
}
