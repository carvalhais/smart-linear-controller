#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Defines.h>

class Bottom
{
public:
    Bottom();

    void begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[]);
    void end();
    void updateVolts(float volts, uint32_t color);
    void updateAmperes(float amps, uint32_t color);
    void updateGain(float gain);
    void updateFan(uint8_t perc);

private:
    void setValue(const char *text, uint16_t x, uint32_t color);
    void drawSubHeader(const char *string, uint16_t x, uint16_t y, uint16_t w);

    uint32_t _x;
    uint32_t _y;
    uint32_t _w;
    uint32_t _h;
    uint8_t _headerHeight = 14;
    uint8_t _meterWidth;
    uint32_t _bgColor = 0x2988;

    TFT_eSPI *_tft;
	TFT_eSprite *_spr;
    uint8_t *_smallFont;
    uint8_t *_mediumFont;
};
