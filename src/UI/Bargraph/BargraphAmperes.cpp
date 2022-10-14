
#include "BargraphAmperes.h"

void BargraphAmperes::setValue(float value)
{
  char valuetxt[5];
  if (value >= 10)
  {
    sprintf(valuetxt, "%1.0f", value);
  }
  else
  {
    sprintf(valuetxt, "%1.1f", value);
  }
  setValueLabel(value / (float)_maxScale, valuetxt);
}

void BargraphAmperes::drawScale()
{
  _spr->loadFont(_smallFont);
  uint8_t steps = 4;
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
