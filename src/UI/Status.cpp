#include "Status.h"

Status::Status()
{
}

void Status::begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, TFT_eSPI *tft, const uint8_t smallFont[], const uint8_t largeFont[])
{
}

void Status::end()
{
}

uint16_t Status::xPos()
{
    return _x;
}
uint16_t Status::yPos()
{
    return _y;
}
uint16_t Status::width()
{
    return _w;
}
uint16_t Status::height()
{
    return _h;
}