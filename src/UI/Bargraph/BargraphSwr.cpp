
#include "BargraphSwr.h"

void BargraphSwr::setReverseMode(ReversePowerMode mode)
{
  _reverseMode = mode;
  if (_spr != nullptr && _spr->created())
  {
    drawScale();
  }
}

void BargraphSwr::setValue(float value)
{
  if (_tft == nullptr) // not initialized
    return;

  if (_reverseMode == MODE_SWR)
  {
    setValueSwr(value);
  }
  else
  {
    BargraphRfPower::setValue(value);
  }
}

void BargraphSwr::drawScale()
{
  if (_tft == nullptr) // not initialized
    return;
  if (_reverseMode == MODE_SWR)
  {
    DBG("SWR: Mode set to SWR\n");
    drawScaleSwr();
  }
  else
  {
    DBG("SWR: Mode set to R. POWER\n");
    BargraphRfPower::drawScale();
  }
}

void BargraphSwr::setValueSwr(float value)
{
  char swrtxt[5];
  float swr = (1 + value) / (1 - value);
  if (swr >= 10)
  {
    sprintf(swrtxt, "%1.0f", swr);
  }
  else
  {
    sprintf(swrtxt, "%1.1f", swr);
  }
  setValueLabel(value, swrtxt);
}

void BargraphSwr::drawScaleSwr()
{
  _spr->loadFont(_smallFont);
  _spr->fillRoundRect(0, _spr->height() - 10, _spr->width(), 10, 2, TFT_BLACK);

  char buf[10];
  float ratio;
  float swrScaleValues[] = {1, 1.5, 2, 3};
  for (uint8_t i = 0; i <= 3; i++)
  {
    float swr = swrScaleValues[i];
    if (_w < 160 && swr == 1.5)
      continue;
    ratio = (swr - 1) / (swr + 1);
    sprintf(buf, "%1.1f", swr);
    auto text = String(buf);
    text.replace(".0", "");
    char *arr = new char[text.length() + 1];
    strcpy(arr, text.c_str());
    drawScaleItem(ratio, arr);
  }
  drawInfiniteSymbol(1);
  BargraphBase::drawScale();
  _spr->unloadFont();
  setValue(0);
}
