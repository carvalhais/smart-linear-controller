#include <Arduino.h>
#include <TFT_eSPI.h>

#include "Types.h"
#include "Bargraph/BargraphSwr.h"
#include "Bargraph/BargraphRfPower.h"
#include "Bargraph/BargraphTemperature.h"

class Meters
{
public:
    Meters();

    void begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[]);
    void end();
    void updateOutputPower(float forwardWatts, float reverseWatts);
    void updateInputPower(float forwardWatts);
    void updateTemperature(float temperature);
    void loop();

private:
    BargraphRfPower _meterInputPower;
    BargraphTemperature _meterTemperature;

    BargraphRfPower _meterOutputPower;
    BargraphSwr _meterOutputSwr;

    time_t _next = millis();
    bool _updated;

    uint32_t _x;
    uint32_t _y;
    uint32_t _w;
    uint32_t _h;

    TFT_eSPI *_tft;
    uint8_t *_smallFont;
    uint8_t *_mediumFont;
};