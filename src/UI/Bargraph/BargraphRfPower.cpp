
#include "BargraphRfPower.h"

void BargraphRfPower::setValue(float value)
{
  if (value > _maxScale && value > (_maxScale * 1.1))
  {
    // increate max scale if higher power is detected
    for (uint8_t i = 1; i < 9; i++)
    {
      if (_scaleValues[i] > value)
      {
        _maxScale = _scaleValues[i];
        setValueLabel(0, (char *)"0");
        drawScale();
        break;
      }
    }
  }

  char wattstxt[5];
  if (!_showDecimal || value == 0 || value >= 100)
  {
    snprintf(wattstxt, 5, "%1.0f", value);
  }
  else
  {
    snprintf(wattstxt, 5, "%1.1f", value);
  }

  if (value > _maxScale)
    value = _maxScale;

  setValueLabel(value / (float)_maxScale, wattstxt);
}

void BargraphRfPower::showDecimal(bool value)
{
  _showDecimal = value;
}

void BargraphRfPower::drawScale()
{
  _spr->loadFont(_smallFont);
  _spr->fillRoundRect(0, _spr->height() - 10, _spr->width(), 10, 2, TFT_BLACK);
  uint8_t steps = 5;
  uint16_t increments = _maxScale / steps;

  for (uint8_t i = 0; i <= steps; i++)
  {
    uint16_t value = i * increments;
    String text = String(value);
    float ratio = (value / (float)_maxScale);
    drawScaleItem(ratio, (char *)text.c_str());
  }

  BargraphBase::drawScale();
  _spr->unloadFont();
  setValue(0);
}

void BargraphRfPower::setInitialScale(uint16_t value)
{
  _maxScale = value;
}