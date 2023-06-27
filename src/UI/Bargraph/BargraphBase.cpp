
#include "BargraphBase.h"

BargraphBase::BargraphBase() {}

BargraphBase::~BargraphBase() {}

void BargraphBase::begin(
    uint32_t x,
    uint32_t y,
    uint32_t w,
    uint32_t h,
    TFT_eSPI *tft,
    const uint8_t smallFont[],
    const uint8_t mediumFont[],
    const char *header)
{
  _x = x;
  _y = y;
  _w = w;
  _h = h;

  _laneX = _paddingLeft;
  _laneY = _paddingTop + _headerHeight + 2;

  _laneW = _w - _laneX - _paddingRight;
  _laneH = _h - _laneY - _paddingBottom;

  _barColorOn = BARGRAPH_COLOR_ON;
  _barColorOff = BARGRAPH_COLOR_OFF;
  _fontColor = BARGRAPH_COLOR_FONT;
  _value = 1.1f;
  _lastValue = 1.1f; // intentionally invalid value, always between 0.0 and 1.0
  _lastValuePosition = 0;
  _lastPeakPosition = 0;
  _header = (char *)header;

  _smallFont = (uint8_t *)smallFont;
  _mediumFont = (uint8_t *)mediumFont;

  uint8_t divider = (_w < 150) ? 30 : 50;
  _numBars = _laneW / (_laneW / divider);
  _barWidth = _laneW / _numBars;

  _laneW = _numBars * _barWidth; // adjust the lane according to the rounded values;

  // DBG("_w: %d, _laneW: %d, _numBars: %d, _barWidth: %d\n", _w, _laneW, _numBars, _barWidth);
  _tft = tft;

  _tft->drawRoundRect(_x, _y, _w, _h, 2, TFT_DARKGREY);
  drawHeader();
  _spr = new TFT_eSprite(tft);
  _spr->setColorDepth(8);
  _spr->createSprite(_w - 4, _h - _laneY - 2);
}

void BargraphBase::end()
{
  if (_spr != nullptr && _spr->created())
  {
    _spr->unloadFont();
    _spr->deleteSprite();
    delete _spr;
    _spr = nullptr;
  }
}

void BargraphBase::setColor(uint32_t on, uint32_t off, uint32_t font)
{
  _barColorOn = on;
  _barColorOff = off;
  _fontColor = font;
}

void BargraphBase::setValueLabel(float value, String valueText)
{
  if (value < 0.0f)
    value = 0;
  if (value > 1.0f)
    value = 1;
  _label = valueText;
  long temp = (long)(value * 1000L);
  _value = (float)temp / 1000.0;
  // DBG("BargraphBase::setValueLabel: %s [Core %d]\n", valueText, xPortGetCoreID());
}

uint8_t BargraphBase::relativePosition(float value)
{
  float currentRelativeValue = _laneW * (value);
  // determine the current position, which bar should be turned on
  return floor(currentRelativeValue / (float)_barWidth);
}

void BargraphBase::loop()
{
  if (_tft == nullptr) // not initialized
    return;

  bool updateScreen = false;
  if (_value != _lastValue)
  {
    updateScreen = true;
    uint8_t start;
    // determine the current position, which bar should be turned on
    uint8_t currentPosition = relativePosition(_value);
    // if the current position is greater than the last position, we need to turn some bars ON
    if (currentPosition > _lastValuePosition && currentPosition > 0)
    {
      start = _lastValuePosition > 0 ? _lastValuePosition : 1;
      for (uint8_t i = start; i <= currentPosition; i++)
      {
        drawBar(i, _barColorOn);
      }
    }

    // if the current position is lower than the last position, we need to turn some bars OFF
    if (currentPosition < _lastValuePosition)
    {
      start = currentPosition > 0 ? currentPosition : 1;
      for (uint8_t i = start; i <= _lastValuePosition; i++)
      {
        drawBar(i, _barColorOff);
      }
    }

    if (_value > _lastValue || _value == 0.0f || millis() > _nextRefresh)
    {
      drawLabelValue();
      _nextRefresh = millis() + 500;
    }

    _lastValue = _value;
    _lastValuePosition = currentPosition;

    if (_value >= _peakValue)
    {
      _peakValue = _value;
      _peakIncrement = 0;
      if (_lastPeakPosition <= currentPosition)
        _lastPeakPosition = 0;
    }
  }

  if (millis() > _timerPeak)
  {
    _timerPeak = millis() + 20;
    if (_peakValue > _value || _lastPeakPosition > 0)
    {
      updateScreen = true;
      uint8_t peakPosition = 0;
      if (_peakValue > 0.0f)
      {
        peakPosition = relativePosition(_peakValue);
      }
      if (_lastPeakPosition != peakPosition)
      {
        if (_lastPeakPosition > 0)
        {
          drawBar(_lastPeakPosition, _barColorOff);
        }
        if (peakPosition > 0)
        {
          drawBar(peakPosition, _barColorOn);
        }
      }

      _lastPeakPosition = peakPosition;
      _peakValue -= (0.001 * _peakIncrement);
      if (_peakValue < _value)
      {
        _peakValue = _value;
      }
      _peakIncrement++;
    }
  }

  if (updateScreen)
  {
    _spr->pushSprite(_x + 2, _y + _laneY);
  }
}

void BargraphBase::drawLabelValue()
{
  // DBG("Bargraph: Text: %s (%d) [Core %d]\n", _valueText, _valueText.length(), xPortGetCoreID());
  uint16_t y = 0;
  uint16_t x = _laneX + _laneW + 2;
  _spr->fillRect(x, y, _w - x - 2, 22, TFT_BLACK);
  _spr->loadFont(_mediumFont);
  _spr->setTextColor(TFT_WHITE, TFT_BLACK);
  x = _w;
  for (uint8_t i = _label.length(); i > 0; i--)
  {
    if (_label.substring(i - 1, i) == ".")
    {
      x -= (_charWidth * .4);
      _spr->drawCentreString(_label.substring(i - 1, i), x - 5, y, 1);
    }
    else
    {
      x -= _charWidth;
      _spr->drawCentreString(_label.substring(i - 1, i), x, y, 1);
    }
  }
  _spr->unloadFont();
}

void BargraphBase::drawBar(uint8_t position, uint32_t color)
{
  _spr->fillRect(_laneX + ((position - 1) * _barWidth) + 1, 0, _barWidth - 1, _laneH, color);
}

void BargraphBase::drawScale()
{
  _spr->drawFastHLine(_laneX, _laneH + 3, _laneW + 1, _barColorOff);
  for (uint8_t i = 1; i <= _numBars; i++)
  {
    drawBar(i, _barColorOff);
  }
  _spr->pushSprite(_x + 2, _y + _laneY);

  _lastValue = -1;
  _peakValue = 0;
  _lastValuePosition = 0;
  _lastPeakPosition = 0;
}

void BargraphBase::drawScaleItem(float value, char *label)
{
  uint16_t x = _laneX + (value * _laneW);
  _spr->drawFastVLine(x, _laneH + 1, 2, _barColorOff);
  _spr->drawCentreString(label, x, _laneH + 5, 1);
  // DBG("_laneX (%d) + (value (%.1f) * _laneW (%d)) = X (%d) - %s\n", _laneX, value, _laneW, x, label);
}

void BargraphBase::drawInfiniteSymbol(float value)
{
  uint16_t x = _laneX + (value * _laneW);
  _spr->drawFastVLine(x, _laneH + 1, 2, _barColorOff);
  _spr->drawCentreString("o", x - 2, _laneH + 4, 1);
  _spr->drawCentreString("o", x + 2, _laneH + 4, 1);
}

void BargraphBase::setHeader(const char *header)
{
  _header = (char *)header;
  drawHeader();
}

void BargraphBase::drawHeader()
{
  _tft->loadFont(_smallFont);
  _tft->fillRect(_x + 5, _y + 1, _w - 10, _headerHeight - 2, TFT_BLACK);
  _tft->drawCentreString(_header, _x + (_w / 2), _y + 3, 1);
  _tft->drawFastHLine(_x, _y + _headerHeight, _w, TFT_DARKGREY);
  _tft->unloadFont();
}
