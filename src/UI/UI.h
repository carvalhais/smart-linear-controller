#ifndef _UI_CLASS_
#define _UI_CLASS_

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Defines.h"
#include "Types.h"
#include <FT62XXTouchScreen.h>
#include "UI/PowerSupplyScreen.h"
#include "UI/StandbyScreen.h"
#include "UI/MainScreen.h"
#include "UI/DiagScreen.h"
#include "Fonts/Tahoma9Sharp.h"
#include "Fonts/EurostileNextProWide13.h"
#include "Fonts/EurostileNextProNr18.h"
#include "Fonts/EurostileNextProSemiBold32.h"
#include "Fonts/EurostileNextProSemiBold26.h"
class UI
{
public:
    UI();
    void begin();
    void loop(bool started);
    void end();

    void previousScreen();
    void nextScreen();
    void loadScreen(Screens screen);
    void updatePowerSupply(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage);
    void updateOutputPower(float forwardWatts, float reverseWatts);
    void updateGain(float gain);
    void updateInputPower(float forwardWatts);
    void updateDiagnostics(Diag diag);
    void updateTemperature(float temperature);
    void updateFanSpeed(float perc);
    void frequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState, char *band, bool txEnabled);
    TouchCmd touch(TouchPoint tp);
    void updateBypass(bool state);
    void setReverseMode(ReversePowerMode mode);
    void unloadScreen(Screens screen);
    void drawHeader();

    MainScreen _main;
    PowerSupplyScreen _psu;
    StandbyScreen _standby;
    DiagScreen _diag;

protected:
private:
    TFT_eSPI _tft = TFT_eSPI();
    ReversePowerMode _reverseMode = ReversePowerMode::MODE_SWR;
    Screens _activeScreen = Screens::STANDBY;
    bool _showReversePower = false;
    ScreenBase *_screenPtr;
    uint8_t *_microFont;
    uint8_t *_smallFont;
    uint8_t *_mediumFont;
    uint8_t *_semiLarge;
    uint8_t *_largeFont;

    uint16_t _x = OFFSET_LEFT + 1;
    uint16_t _y = OFFSET_TOP + HEADER_HEIGHT + 1;
    uint16_t _w = SCREEN_WIDTH - 2;
    uint16_t _h = SCREEN_HEIGHT - HEADER_HEIGHT - 2;
};

#endif