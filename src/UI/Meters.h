#include <Arduino.h>
#include <TFT_eSPI.h>

#include "UI.h"
#include "Bargraph/BargraphSwr.h"
#include "Bargraph/BargraphRfPower.h"

class Meters : public UI
{
public:
    Meters();

    void begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[]);
    void end();
    void updateInputSwr(float forwardMv, float reverseMv);
    void updateOutputSwr(float forwardMv, float reverseMv);
    void loop();

private:
    
    void drawSubHeader(const char *string, uint16_t x, uint16_t y, uint16_t w);

    BargraphRfPower _meterInputPower;
    BargraphSwr _meterInputSwr;

    BargraphRfPower _meterOutputPower;
    BargraphSwr _meterOutputSwr;

    time_t _next = millis();

    uint32_t _x;
    uint32_t _y;
    uint32_t _w;
    uint32_t _h;

    uint8_t *_smallFont;
};