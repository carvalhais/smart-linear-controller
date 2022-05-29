#include "AnalogMeter.h"

AnalogMeter::AnalogMeter() {}

void AnalogMeter::begin(TFT_eSPI *tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    _tft = tft;
    _x = x;
    _y = y;
    _faceWidth = width;
    _faceHeight = height;
    _spr = std::unique_ptr<TFT_eSprite>(new TFT_eSprite(tft));
    _spr->createSprite(_faceWidth, _faceHeight);
    drawScale(_minAngle, _maxAngle);
    _spr->pushSprite(_x, _y);
    update(_value); // Put meter needle at 0
}

void AnalogMeter::update(int value)
{
    if (value > 100)
        value = 100;
    if (value < 0)
        value = 0;
    _value = value;
}

void AnalogMeter::loop()
{
    if (_spr == nullptr)
        return;

    if (millis() > _timer1)
    {
        uint8_t speed = 10;
        if (_value == _currentValue)
        {
            return;
        }
        else if (_value > _currentValue)
        {
            _currentValue += 4;
            if (_currentValue > _value)
                _currentValue = _value;
        }
        else if (_value < _currentValue)
        {
            speed = 60;
            _currentValue -= 2;
            if (_currentValue < _value)
                _currentValue = _value;
        }

        _timer1 = millis() + speed;

        if (_oldAngle != 999)
        {
            eraseNeedle(_oldAngle);
        }

        int sdeg = map(_currentValue, 0, 100, _minAngle, _maxAngle); // Map value to angle
        _oldAngle = sdeg;
        drawNeedle(sdeg);
        _spr->pushSprite(_x, _y);
    }
}

void AnalogMeter::drawScale(int from, int to)
{
    if (to > _maxAngle)
        to = _maxAngle;
    if (from < _minAngle)
        from = _minAngle;

    from = floor(from / 5.0) * 5;
    to = ceil(to / 5.0) * 5;

    // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
    for (int i = from; i <= to; i += 5)
    {
        int perc = map(i, _minAngle, _maxAngle, 0, 100);

        uint32_t color = perc >= 60 ? _redZone : _scaleColor;

        // Coodinates of tick to draw
        float sx = cos(i * DEG2RAD);
        float sy = sin(i * DEG2RAD);

        uint16_t x0 = (sx * (_radiusX + _longTick)) + _pivotX;
        uint16_t y0 = (sy * (_radiusY + _longTick)) + _pivotY;
        uint16_t x1 = (sx * _radiusX) + _pivotX;
        uint16_t y1 = (sy * _radiusY) + _pivotY;

        // Coordinates of next tick for zone fill
        float sx2 = cos((i + 5) * DEG2RAD);
        float sy2 = sin((i + 5) * DEG2RAD);

        // int x2 = (sx2 * (_radiusX + _longTick)) + _pivotX;
        // int y2 = (sy2 * (_radiusY + _longTick)) + _pivotY;

        // int x3 = (sx2 * _radiusX) + _pivotX;
        // int y3 = (sy2 * _radiusY) + _pivotY;

        // // Green zone limits
        // if (i >= -140 && i < -90)
        // {
        //     _spr->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
        //     _spr->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
        // }

        // // Yellow zone limits
        // if (i >= -90 && i < -65)
        // {
        //     _spr->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
        //     _spr->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
        // }

        // // Orange zone limits
        // if (i >= -65 && i < -40)
        // {
        //     _spr->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
        //     _spr->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
        // }

        // Long scale tick length
        int tl = perc % 25 != 0 ? _shortTick : _longTick;

        // Recalculate coords incase tick lenght changed
        x0 = sx * (_radiusX + tl) + _pivotX;
        y0 = sy * (_radiusY + tl) + _pivotY;

        // Draw tick
        _spr->drawWideLine(x0, y0, x1, y1, 2, color);

        // Check if labels should be drawn, with position tweaks
        if (perc % 25 == 0)
        {
            drawLabel(sx, sy, perc);
        }

        // Draw scale arc, don't draw the last part
        if (i < _maxAngle)
        {
            x0 = (sx2 * _radiusX) + _pivotX;
            y0 = (sy2 * _radiusY) + _pivotY;
            _spr->drawWideLine(x0, y0, x1, y1, 2, color);
        }
    }
}

void AnalogMeter::drawLabel(float sx, float sy, int deg)
{
    // Calculate label positions
    uint16_t x0 = sx * (_radiusX + _longTick + 10) + _pivotX;
    uint16_t y0 = sy * (_radiusY + _longTick + 10) + _pivotY;

    // switch (deg / 25)
    // {
    // case 0:
    //     _spr->drawCentreString("0", x0, y0 - 15, 2);
    //     break;
    // case 1:
    //     _spr->drawCentreString("25", x0, y0 - 9, 2);
    //     break;
    // case 2:
    //     _spr->drawCentreString("50", x0, y0 - 6, 2);
    //     break;
    // case 3:
    //     _spr->drawCentreString("75", x0, y0 - 9, 2);
    //     break;
    // case 4:
    //     _spr->drawCentreString("100", x0, y0 - 12, 2);
    //     break;
    // }
}

void AnalogMeter::drawNeedle(int sdeg)
{
    float sx = cos(sdeg * DEG2RAD);
    float sy = sin(sdeg * DEG2RAD);
    float ty = tan((90 - sdeg) * DEG2RAD); // we need the inverse angle

    int x0 = sx * (_radiusX + 8) + _pivotX;
    int y0 = sy * (_radiusY + 8) + _pivotY;

    int x1 = _pivotX - (_needleDelta * ty);
    int y1 = _pivotY - _needleDelta;

    _spr->drawWedgeLine(x0, y0, x1, y1, 1, 3, _needleColor, _backgroundColor);
    //_spr->drawWideLine(x0, y0, x1, y1, 2, _needleColor, _backgroundColor);
    //_spr->drawLine(x0, y0, x1, y1, _needleColor);
}

void AnalogMeter::eraseNeedle(int sdeg)
{
    int needleDelta = _needleDelta - 3;
    int degressOffsetTop = 1;
    int degressOffsetBottom = 3;

    float sx0 = cos((sdeg - degressOffsetTop) * DEG2RAD);
    float sy0 = sin((sdeg - degressOffsetTop) * DEG2RAD);

    float sx1 = cos((sdeg + degressOffsetTop) * DEG2RAD);
    float sy1 = sin((sdeg + degressOffsetTop) * DEG2RAD);

    float ty0 = tan((90 - sdeg + degressOffsetBottom) * DEG2RAD); // 90-degree = oposite angle
    float ty1 = tan((90 - sdeg - degressOffsetBottom) * DEG2RAD); // 90-degree = oposite angle

    // calculate top left position
    float x0 = sx0 * (_radiusX + 10) + _pivotX;
    float y0 = sy0 * (_radiusY + 10) + _pivotY;

    // calculate the top right position
    float x1 = sx1 * (_radiusX + 10) + _pivotX;
    float y1 = sy1 * (_radiusY + 10) + _pivotY;

    // calculate bottom left position
    int x2 = _pivotX - (needleDelta * ty0);
    int y2 = _pivotY - needleDelta;

    // calculate bottom right position
    int x3 = _pivotX - (needleDelta * ty1);
    int y3 = _pivotY - needleDelta;

    _spr->fillTriangle(x0, y0, x1, y1, x2, y2, _backgroundColor);
    _spr->fillTriangle(x1, y1, x2, y2, x3, y3, _backgroundColor);

    drawScale(sdeg - degressOffsetTop, sdeg + degressOffsetTop);
}
