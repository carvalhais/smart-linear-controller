
#include "BargraphRfPower.h"

void BargraphRfPower::setValue(float value)
{
  if (value > _maxScale)
  {
    // increate max scale if higher power is detected
    for (uint8_t i = 1; i < 7; i++)
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
  if (value >= 10)
  {
    sprintf(wattstxt, "%1.0f", value);
  }
  else
  {
    sprintf(wattstxt, "%1.1f", value);
  }
  setValueLabel(value / (float)_maxScale, wattstxt);
}

void BargraphRfPower::drawScale()
{
  _spr->loadFont(_smallFont);
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
  setValue(0.1);
  setValue(0);
}
