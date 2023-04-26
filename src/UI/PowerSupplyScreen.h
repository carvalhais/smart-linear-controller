#include <Arduino.h>
#include <TFT_eSPI.h>
#include "CAN/CANTypes.h"
#include "UI/Bargraph/BargraphAmperes.h"
#include "ScreenBase.h"
#include "SimpleMeter.h"

class PowerSupplyScreen : public ScreenBase
{
public:
    PowerSupplyScreen();

    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TFT_eSPI *tft, const uint8_t microFont[], const uint8_t smallFont[], const uint8_t mediumFont[], const uint8_t largeFont[]);
    void setStatus(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage);
    void updateRFOutputPower(float power);
    void end();
    void loop();

private:
    void setMode(const char *mode, uint32_t color);
    time_t _next = millis();
    TFT_eSPI *_tft;

    BargraphAmperes _bargraph;
    uint8_t *_microFont;
    uint8_t *_smallFont;
    uint8_t *_mediumFont;
    uint8_t *_largeFont;
    uint16_t _x;
    uint16_t _y;
    uint16_t _w;
    uint16_t _h;
    uint8_t _meterWidth;
    uint8_t _headerHeight = 20;
    uint8_t _meterHeight = 40;

    PowerSupplyMode _mode = PowerSupplyMode::NOT_STARTED;
    PowerSupplyMode _lastMode = PowerSupplyMode::NOT_STARTED;
    int _intakeTemperature;
    float _current;
    float _outputVoltage;
    int _inputVoltage;
    int _outputTemperature;
    bool _update = false;
    float _rfOutputPower;
    bool _init = false;

    SimpleMeter _meterDCOut;
    SimpleMeter _meterACIn;
    SimpleMeter _meterTempIn;
    SimpleMeter _meterTempOut;
    SimpleMeter _meterCurrent;
    SimpleMeter _meterDCPower;
    SimpleMeter _meterRFPower;
    SimpleMeter _meterEfficiency;

};