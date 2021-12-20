#include <Arduino.h>
#include <TFT_eSPI.h>
#include "UI.h"

class Bottom : public UI
{
public:
    Bottom();

    void begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[]);
    void end();
    void updateVolts(float volts);
    void updateAmperes(float amps);
    void updateTemperature(float temp);
    void updateAntenna(char *ant);

private:
    void setValue(char *text, uint16_t x, uint16_t y);
    void drawSubHeader(const char *string, uint16_t x, uint16_t y, uint16_t w);

    uint32_t _x;
    uint32_t _y;
    uint32_t _w;
    uint32_t _h;

};