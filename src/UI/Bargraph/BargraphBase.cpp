
#include "BargraphBase.h"

BargraphBase::BargraphBase() {}

BargraphBase::~BargraphBase() {}

void BargraphBase::begin(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t mediumFont[])
{
  _x = x;
  _y = y;
  _w = w;
  _h = h;

  _laneX = _paddingLeft;
  _laneY = _paddingTop;
  _laneW = _w - _laneX - _paddingRight;
  _laneH = _h - _laneY - _paddingBottom;

  _barColorOn = BARGRAPH_COLOR_ON;
  _barColorOff = BARGRAPH_COLOR_OFF;
  _fontColor = BARGRAPH_COLOR_FONT;
  _value = 0;
  _lastValue = 0;
  _lastValuePosition = 0;
  _lastPeakPosition = 0;

  _smallFont = (uint8_t *)smallFont;
  _mediumFont = (uint8_t *)mediumFont;

  uint8_t divider = (_w < 150) ? 30 : 50;
  _numBars = _laneW / (_laneW / divider);
  _barWidth = _laneW / _numBars;

  //Serial.printf("_w: %d, _laneW: %d, _numBars: %d, _barWidth: %d\n", _w, _laneW, _numBars, _barWidth);

  _tft = tft;
  _spr = std::unique_ptr<TFT_eSprite>(new TFT_eSprite(tft));

  _spr->createSprite(_w, _h);
  _spr->drawRoundRect(0, 0, _w, _h, 2, TFT_DARKGREY);
}

void BargraphBase::end()
{
  if (_spr != nullptr && _spr->created())
  {
    _spr->unloadFont();
    _spr->deleteSprite();
  }
}

void BargraphBase::setColor(uint32_t on, uint32_t off, uint32_t font)
{
  _barColorOn = on;
  _barColorOff = off;
  _fontColor = font;
}

void BargraphBase::setValueLabel(float value, char *valueText)
{
  if (value < 0.0)
    value = 0;
  if (value > 1.0)
    value = 1;
  _valueText = valueText;
  long temp = (long)(value * 1000L);
  _value = (float)temp / 1000.0;
  loop();
}

void BargraphBase::loop()
{
  if (_tft == nullptr) // not initialized
    return;

  bool updateScreen = false;

  if (_value != _lastValue)
  {
    updateScreen = true;
    uint8_t start = 0;
    float currentRelativeValue = _laneW * (_value);

    //determine the current position, which bar should be turned on
    uint8_t currentPosition = floor(currentRelativeValue / (float)_barWidth);

    //Serial.printf("Value %1.2f, Position: %d\n", _value, currentPosition);

    //if the current position is greater than the last position, we need to turn some bars ON
    if (currentPosition > _lastValuePosition && currentPosition > 0)
    {
      start = _lastValuePosition > 0 ? _lastValuePosition : 1;
      for (uint8_t i = start; i <= currentPosition; i++)
      {
        drawBar(i, _barColorOn);
      }
    }

    //if the current position is lower than the last position, we need to turn some bars OFF
    if (currentPosition < _lastValuePosition)
    {
      start = currentPosition > 0 ? currentPosition : 1;
      for (uint8_t i = start; i <= _lastValuePosition; i++)
      {
        drawBar(i, _barColorOff);
      }
    }
    _lastValue = _value;
    _lastValuePosition = currentPosition;
    if (_value > _peakValue)
    {
      _peakValue = _value;
      _peakIncrement = 0;
      if (_lastPeakPosition <= currentPosition)
        _lastPeakPosition = 0;
    }
    drawLabelValue();
  }

  if (_peakValue > _value || _lastPeakPosition > 0)
  {
    updateScreen = true;
    uint8_t barPosition = 0;
    if (_peakValue > 0)
    {
      float peakRelativeValue = _laneW * (_peakValue);
      barPosition = floor(peakRelativeValue / (float)_barWidth);
    }

    if (_lastPeakPosition != barPosition)
    {
      if (_lastPeakPosition > 0)
      {
        drawBar(_lastPeakPosition, _barColorOff);
      }
      if (barPosition > 0)
      {
        drawBar(barPosition, _barColorOn);
      }
    }
    _lastPeakPosition = barPosition;
    _peakValue = _peakValue - (0.02 * _peakIncrement);

    if (_peakValue < _value)
    {
      _peakValue = _value;
    }
    _peakIncrement++;
  }

  if (updateScreen)
  {
    _spr->pushSprite(_x, _y);
  }
}

void BargraphBase::drawLabelValue()
{
  uint16_t y = (_h / 2) - 11;
  _spr->fillRect(_w - _paddingRight + 3, y, 32, 16, TFT_BLACK);
  _spr->loadFont(_mediumFont);
  String str = String(_valueText);
  uint16_t x = _w - 2;
  for (uint8_t i = str.length(); i > 0; i--)
  {
    if (str.substring(i - 1, i) == ".")
    {
      x -= 5;
      _spr->drawCentreString(str.substring(i - 1, i), x - 2, y, 1);
    }
    else
    {
      x -= 9;
      _spr->drawCentreString(str.substring(i - 1, i), x, y, 1);
    }
  }
  _spr->unloadFont();
}

void BargraphBase::drawBar(uint8_t position, uint32_t color)
{
  _spr->fillRect(_laneX + ((position - 1) * _barWidth) + 1, _laneY, _barWidth - 1, _laneH, color);
}

void BargraphBase::drawScale()
{
  _spr->drawFastHLine(_laneX, _laneY + _laneH + 3, _laneW, _barColorOff);
  for (uint8_t i = 1; i <= _numBars; i++)
  {
    drawBar(i, _barColorOff);
  }
  _spr->pushSprite(_x, _y);
}

void BargraphBase::drawScaleItem(float value, char *label)
{
  uint8_t x = _laneX + (value * _laneW);
  _spr->drawFastVLine(x, _laneY + _laneH + 1, 2, _barColorOff);
  _spr->drawCentreString(label, x, _laneY + _laneH + 5, 1);
}

void BargraphBase::drawInfiniteSymbol(float value)
{
  uint8_t x = _laneX + (value * _laneW);
  _spr->drawFastVLine(x, _laneY + _laneH + 1, 2, _barColorOff);
  _spr->drawCentreString("o", x - 2, _laneY + _laneH + 4, 1);
  _spr->drawCentreString("o", x + 2, _laneY + _laneH + 4, 1);
}