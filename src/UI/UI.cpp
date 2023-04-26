#include "UI.h"

UI::UI()
{
}

void UI::drawHeader()
{
    uint16_t width = SCREEN_WIDTH / 2;
    for (uint16_t i = 0; i <= width; i++)
    {
        uint8_t alpha = (255 * i) / width; // alpha is a value in the range 0-255
        uint16_t color = _tft.alphaBlend(alpha, 0x4A69, TFT_BLACK);
        _tft.drawFastVLine(OFFSET_LEFT + i, OFFSET_TOP, HEADER_HEIGHT, color);
        _tft.drawFastVLine(OFFSET_LEFT + SCREEN_WIDTH - i, OFFSET_TOP, HEADER_HEIGHT, color);
    }
    _tft.loadFont(_smallFont);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(TFT_WHITE, 0x4A69, false);
    _tft.drawString(HEADER_TITLE, OFFSET_LEFT + (SCREEN_WIDTH / 2), OFFSET_TOP + (HEADER_HEIGHT / 2) + 2);
    _tft.drawFastHLine(OFFSET_LEFT, OFFSET_TOP + HEADER_HEIGHT, SCREEN_WIDTH, TFT_DARKGREY);
    _tft.drawRect(OFFSET_LEFT, OFFSET_TOP, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_DARKGREY);
}

void UI::begin()
{
    _microFont = (uint8_t *)Tahoma9Sharp;
    _smallFont = (uint8_t *)EurostileNextProWide13;
    _mediumFont = (uint8_t *)EurostileNextProNr18;
    _semiLarge = (uint8_t *)EurostileNextProSemiBold26;
    _largeFont = (uint8_t *)EurostileNextProSemiBold32;

    _tft.init();
#ifdef TFT_ROTATION
    _tft.setRotation(TFT_ROTATION);
#endif
    _tft.fillScreen(TFT_BLACK);
    drawHeader();
}

void UI::loop(bool started)
{
    switch (_activeScreen)
    {
    case Screens::MAIN:
        if (started)
            _main.loop();
        break;
    case Screens::PSU:
        _psu.loop();
        break;
    case Screens::DIAG:
        _diag.loop();
        break;
    case Screens::STANDBY:
        // Nothing to do, just to let compiler happy
        break;
    }
    //_analog.loop();
}

void UI::previousScreen()
{
    switch (_activeScreen)
    {
    case Screens::MAIN:
        loadScreen(Screens::PSU);
        break;
    case Screens::PSU:
        loadScreen(Screens::MAIN);
        break;
    case Screens::STANDBY:
        // Nothing to do, just to let compiler happy
        break;
    }
}

void UI::nextScreen()
{

    switch (_activeScreen)
    {
    case Screens::MAIN:
        loadScreen(Screens::PSU);
        break;
    case Screens::PSU:
        loadScreen(Screens::MAIN);
        break;
    case Screens::STANDBY:
        // Nothing to do, just to let compiler happy
        break;
    }
}

void UI::loadScreen(Screens screen)
{
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);

    if (screen != _activeScreen)
    {
        unloadScreen(_activeScreen);
    }

    switch (screen)
    {
    case Screens::MAIN:
        DBG("Active Screen: MAIN\n");
        _screenPtr = &_main;
        _main.begin(_x, _y, _w, _h, &_tft, _microFont, _smallFont, _mediumFont, _semiLarge, _largeFont);
        break;
    case Screens::PSU:
        DBG("Active Screen: PSU\n");
        _screenPtr = &_psu;
        _psu.begin(_x, _y, _w, _h, &_tft, _microFont, _smallFont, _semiLarge, _largeFont);
        break;
    case Screens::STANDBY:
        DBG("Active Screen: STANDBY\n");
        _screenPtr = &_standby;
        _standby.begin(_x, _y, _w, _h, &_tft, _smallFont);
        break;
    case Screens::DIAG:
        DBG("Active Screen: DIAG\n");
        _screenPtr = &_diag;
        _diag.begin(_x, _y, _w, _h, &_tft, _microFont, _smallFont, _mediumFont, _largeFont);
        break;
    }
    _activeScreen = screen;
}

void UI::updatePowerSupply(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage)
{
    _psu.setStatus(mode, intTemp, outTemp, current, outVoltage, inputVoltage);
    _diag.updatePowerSupply(mode, intTemp, outTemp, current, outVoltage, inputVoltage);
    _main.updatePowerSupply(mode, intTemp, outTemp, current, outVoltage, inputVoltage);
}

void UI::updateOutputPower(float forwardWatts, float reverseWatts)
{
    switch (_activeScreen)
    {
    case Screens::MAIN:
        _main.updateOutputPower(forwardWatts, reverseWatts);
        break;
    case Screens::PSU:
        _psu.updateRFOutputPower(forwardWatts);
        break;
    }
}

void UI::updateGain(float gain)
{
    if (_activeScreen == Screens::MAIN)
    {
        _main.updateGain(gain);
    }
}

void UI::updateInputPower(float forwardWatts)
{
    if (_activeScreen == Screens::MAIN)
    {
        _main.updateInputPower(forwardWatts);
    }
}

void UI::updateDiagnostics(Diag diag)
{
    _diag.update(diag);
}

void UI::updateTemperature(float temperature)
{
    if (_activeScreen == Screens::MAIN)
    {
        _main.updateTemperature(temperature);
    }
}

void UI::updateFanSpeed(float perc)
{
    if (_activeScreen == Screens::MAIN)
    {
        _main.updateFanSpeed(perc);
    }
}

void UI::frequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState, char *band, bool txEnabled)
{
    _main.frequencyChanged(frequency, modulation, filter, txState, band, txEnabled);
}

TouchCmd UI::touch(TouchPoint tp)
{
    DBG("UI::touch: X: %d, Y: %d\n", tp.xPos, tp.yPos);
    return _screenPtr->getTouchRegion(tp);
}

void UI::updateBypass(bool state)
{
    if (_activeScreen == Screens::MAIN)
    {
        _main.setByPassState(state);
    }
}

void UI::unloadScreen(Screens screen)
{
    switch (screen)
    {
    case Screens::MAIN:
        DBG("Unloading Screen: MAIN\n");
        _main.end();
        break;
    case Screens::PSU:
        DBG("Unloading Screen: PSU\n");
        _psu.end();
        break;
    case Screens::STANDBY:
        DBG("Unloading Screen: STANDBY\n");
        break;
    case Screens::DIAG:
        DBG("Unloading Screen: DIAG\n");
        _diag.end();
    }
}

void UI::end()
{
    unloadScreen(_activeScreen);
}
