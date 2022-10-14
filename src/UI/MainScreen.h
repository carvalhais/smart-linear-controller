#include <Arduino.h>
#include <TFT_eSPI.h>
#include "UI/Frequency.h"
#include "UI/Meters.h"
#include "UI/Bottom.h"
#include "UI/GradientButton.h"
#include "CAN/CANTypes.h"
#include "ScreenBase.h"

class MainScreen : public ScreenBase
{
public:
    MainScreen();
    void begin(
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        TFT_eSPI *tft,
        const uint8_t microFont[],
        const uint8_t smallFont[],
        const uint8_t mediumFont[],
        const uint8_t largeFont[]);

    void loop();
    void end();

    void updateOutputPower(float forwardWatts, float reverseWatts);
    void updateGain(float gain);
    void updateInputPower(float forwardWatts);
    void frequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState, char *band, bool txEnabled);
    void updatePowerSupply(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage);
    void updateTemperature(float temperature);
    void updateFanSpeed(uint8_t perc);
    void setByPassState(bool state);

    Frequency _freq;
    Meters _meters;
    Bottom _bottom;

private:
    TFT_eSPI *_tft;
    uint8_t *_microFont;
    uint8_t *_smallFont;
    uint8_t *_mediumFont;
    uint8_t *_largeFont;
    uint16_t _x;
    uint16_t _y;
    uint16_t _w;
    uint16_t _h;
    uint8_t _bandHeight;
    
    GradientButton _btnBypass;
    GradientButton _btnPowerOff;

    PowerSupplyMode _mode = PowerSupplyMode::NOT_STARTED;
    int _intakeTemperature;
    float _current;
    float _outputVoltage;
    int _inputVoltage;
    int _outputTemperature;
    bool _update = false;
};