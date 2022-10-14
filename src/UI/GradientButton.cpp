#include "GradientButton.h"

GradientButton::GradientButton()
{
}

uint16_t GradientButton::xPos()
{
  return _x;
}
uint16_t GradientButton::yPos()
{
  return _y;
}
uint16_t GradientButton::width()
{
  return _w;
}
uint16_t GradientButton::height()
{
  return _h;
}

void GradientButton::begin(TFT_eSPI *tft, const uint8_t font[], uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t radius)
{
  _x = x;
  _y = y;
  _w = w;
  _h = h;
  _radius = radius;
  _spr = new TFT_eSprite(tft);
  _spr->setColorDepth(16);
  _spr->createSprite(_w, _h);
  _spr->loadFont(font);
  _spr->setTextDatum(TC_DATUM);
}

void GradientButton::end()
{
  if (_spr != nullptr && _spr->created())
  {
    _spr->deleteSprite();
    delete _spr;
    _spr = nullptr;
  }
}

void GradientButton::setBackColors(uint16_t topColor, uint16_t bottomColor, uint16_t borderColor)
{
  _topColor = topColor;
  _bottomColor = bottomColor;
  _borderColor = borderColor;
}

void GradientButton::setDashed(bool dashed)
{
  _dashed = dashed;
}

void GradientButton::setText(uint16_t textColor, const char *text)
{
  _textColor = textColor;
  _text = text;
}

void GradientButton::update()
{
  _spr->fillSprite(TFT_BLACK);
  // draw the border once to use as a area delimiter
  _spr->drawRoundRect(0, 0, _w, _h, _radius, _borderColor);

  // detect the borders and write a horizontal line according to the gradient pattern
  for (uint16_t n = 0; n < _h; n++)
  {
    for (uint16_t z = 0; z < _radius; z++)
    {
      uint16_t px = _spr->readPixel(z, n);
      if (px != 0)
      {
        uint8_t alpha = (255 * n) / _h; // alpha is a value in the range 0-255
        uint16_t color = _spr->alphaBlend(alpha, _bottomColor, _topColor);
        uint16_t xs = z + 1;
        uint16_t xe = _w - z - 1;
        _spr->drawFastHLine(xs, n, xe - xs, color);
        break;
      }
    }
  }
  // draw de border again to ensure smoothnesse
  _spr->drawRoundRect(0, 0, _w, _h, _radius, _borderColor);
  _spr->setTextColor(_textColor, _bottomColor);
  _spr->setTextDatum(MC_DATUM);
  _spr->drawString(_text, (_w / 2), (_h / 2) + 1, 1);

  if (_dashed)
  {
    uint8_t dashWidth = 2;
    uint8_t offset = _radius + dashWidth;
    uint8_t numDashesH = (_w - offset) / dashWidth / 2;
    uint8_t numDashesV = (_h - offset) / dashWidth / 2;

    for (uint8_t i = 0; i < numDashesH; i++)
    {
      uint8_t pos = (i * 2) * dashWidth;
      _spr->drawFastHLine(offset + pos, 0, dashWidth, TFT_BLACK);
      _spr->drawFastHLine(offset + pos, _h - 1, dashWidth, TFT_BLACK);
    }
    for (uint8_t i = 0; i < numDashesV; i++)
    {
      uint8_t pos = (i * 2) * dashWidth;
      _spr->drawFastVLine(0, offset + pos, dashWidth, TFT_BLACK);
      _spr->drawFastVLine(_w - 1, offset + pos, dashWidth, TFT_BLACK);
    }
  }

  _spr->pushSprite(_x, _y);
}
