#include <Arduino.h>
#include <TFT_eSPI.h>
#include <UI/GradientButton.h>

class Frequency
{
public:
    Frequency();

    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[], const uint8_t largeFont[]);
    void end();
    void frequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState, char *band, bool txEnabled);
    void setTransmitEnabled(bool state);
    void loop();

private:
    uint16_t _x;
    uint16_t _y;
    uint16_t _w;
    uint16_t _h;

    uint8_t *_smallFont;
    uint8_t *_mediumFont;
    uint8_t *_largeFont;
    TFT_eSPI *_tft;
    TFT_eSprite *_spr;


    uint8_t _charWidth = 20;
    uint8_t _offsetTop = 0;

    GradientButton _btnTx;
    GradientButton _btnMode;
    GradientButton _btnBand;
    uint32_t _frequency;
    uint8_t _modulation;
    uint8_t _filter;

    bool _txState;
    char *_band;
    bool _freqChanged;
    bool _txEnabled;
    bool _bandChanged;
    bool _txStateChanged; // draw the button once;
    bool _modChanged;
    bool _updated;

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
    const char *_filterText[4] = {"NONE", "FIL1", "FIL2", "FIL3"};
};