#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Defines.h"
class AnalogMeter
{
public:
    AnalogMeter();

    void begin(TFT_eSPI *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void update(int value);
    int value() { return _value; }
    void loop();

private:
    void drawScale(int from, int to);
    void drawLabel(float sx, float xy, int deg);
    void drawNeedle(int sdeg);
    void eraseNeedle(int sdeg);

    TFT_eSPI *_tft;
    std::unique_ptr<TFT_eSprite> _spr;

    int _x;
    int _y;
    int _faceWidth = 320;
    int _faceHeight = 120;

    const int _needleLength = _faceHeight / 2;
    const int _radiusX = _faceHeight + 20;
    const int _radiusY = _faceHeight + 20;
    const int _offsetTop = 40;

    const int _pivotX = _faceWidth / 2;
    const int _pivotY = _radiusY + _offsetTop;

    const int _minAngle = -130;
    const int _maxAngle = -50;

    const int _longTick = 15;
    const int _shortTick = 8;

    const int _needleDelta = 80;

    uint32_t _backgroundColor = TFT_BLACK;
    uint32_t _needleColor = TFT_WHITE;
    uint32_t _scaleColor = TFT_SILVER;
    uint32_t _redZone = TFT_RED;

    int _value = 0;
    int _currentValue = 0;
    int _oldAngle = 999;

    volatile time_t _timer1;
};