#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Types.h>
#include <Defines.h>
#include <CAN/CANTypes.h>
#include "ScreenBase.h"

class DiagScreen : public ScreenBase
{
public:
    DiagScreen();
    void begin(
        uint32_t x,
        uint32_t y,
        uint32_t w, uint32_t h,
        TFT_eSPI *tft,
        const uint8_t microFont[],
        const uint8_t smallFont[],
        const uint8_t mediumFont[],
        const uint8_t largeFont[]);

    void update(Diag diag);
    void updatePowerSupply(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage);
    void loop();
    void end();

private:
    void printItem(String text, bool value);
    void printDetail(String text, String value);
    TFT_eSPI *_tft;
    uint8_t *_microFont;
    uint8_t *_smallFont;
    uint8_t *_mediumFont;
    uint8_t *_largeFont;
    uint32_t _x;
    uint32_t _y;
    uint32_t _w;
    uint32_t _h;
    uint8_t _colWidth = 200;
    uint8_t _padLeft = 10;
    uint8_t _update;
    Diag _diag;
    PowerSupplyMode _mode;
    String _modeText;
    uint8_t _position;
};