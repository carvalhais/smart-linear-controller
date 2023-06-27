#include "TouchRegion.h"

TouchRegion::TouchRegion()
{
}

TouchRegion::TouchRegion(IWidget &widget, TouchCmd cmd)
{
    _cmd = cmd;
    setCoords(widget.xPos(), widget.yPos(), widget.width(), widget.height());
}

TouchRegion::TouchRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TouchCmd cmd)
{
    _cmd = cmd;
    setCoords(x, y, w, h);
}

void TouchRegion::drawOutline(TFT_eSPI *tft, uint32_t color)
{
    tft->drawRect(_x, _y, _w, _h, color);
}

void TouchRegion::setCoords(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    //DBG("TouchRegion: X: %d, Y: %d, W: %d, H: %d, CMD: %s\n", x, y, w, h, _touchCmdText[_cmd]);
    _x = (x - _border) <= 0 ? 0 : x - _border;
    _y = (y - _border) <= 0 ? 0 : y - _border;
    _w = (w + _border * 2) >= SCREEN_WIDTH ? SCREEN_WIDTH : w + _border * 2;
    _h = (h + _border * 2) >= SCREEN_HEIGHT ? SCREEN_HEIGHT : h + _border * 2;
    //DBG("TouchRegion (w/ borders): X: %d, Y: %d, W: %d, H: %d, CMD: %s\n", _x, _y, _w, _h, _touchCmdText[_cmd]);
}

uint16_t TouchRegion::xPos()
{
    //DBG("TouchRegion: xPos(): X: %d, Y: %d, W: %d, H: %d\n", _x, _y, _w, _h);
    return _x;
}

uint16_t TouchRegion::yPos()
{
    return _y;
}
uint16_t TouchRegion::width()
{
    return _w;
}
uint16_t TouchRegion::height()
{
    return _h;
}
TouchCmd TouchRegion::command()
{
    return _cmd;
}