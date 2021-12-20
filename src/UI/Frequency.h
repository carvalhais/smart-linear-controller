#include <Arduino.h>
#include <TFT_eSPI.h>

class Frequency
{
public:
    Frequency();

    void begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[], const uint8_t largeFont[]);
    void end();
    void frequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState, char *band, bool txEnabled);
    void loop();

private:
    void gradientButton(uint16_t x, uint16_t y, uint8_t w, uint8_t h, uint8_t radius, uint16_t topColor, uint16_t bottomColor, uint16_t borderColor, bool dashed);

    uint32_t _x;
    uint32_t _y;
    uint32_t _w;
    uint32_t _h;

    uint8_t *_smallFont;
    uint8_t *_mediumFont;
    uint8_t *_largeFont;
    TFT_eSPI *_tft;
    std::unique_ptr<TFT_eSprite> _spr;

    uint32_t _frequency;
    uint8_t _modulation;
    uint8_t _filter;
    bool _txState;
    char *_band;
    bool _freqChanged;
    bool _txEnabled;

    /*
    00 : LSB 
    01 : USB 
    02 : AM 
    03 : CW 
    04 : RTTY 
    05 : FM 
    06 : WFM 
    07 : CW-R 
    08 : RTTY-R 
    17 : DV
*/
    const char *_modeText[9] = {"LSB", "USB", "AM", "CW", "RTTY", "FM", "WFM", "CW-R", "RTTY-R"};
    const char *_filterText[4] = {"Unknown", "FIL1", "FIL2", "FIL3"};
};