
#include "BargraphTemperature.h"

void BargraphTemperature::setValue(float value)
{
  char valuetxt[5];
  sprintf(valuetxt, "%1.1f", value);
  setValueLabel(value / (float)_maxScale, valuetxt);
}

void BargraphTemperature::drawScale()
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
