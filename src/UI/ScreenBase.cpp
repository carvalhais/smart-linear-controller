#include "ScreenBase.h"

ScreenBase::ScreenBase()
{
}

TouchCmd ScreenBase::getTouchRegion(TouchPoint tp)
{
    uint8_t touchBorder = 10;
    for (uint8_t i = 0; i < _numTouchRegions; i++)
    {
        TouchRegion reg = _touchRegions[i];

        uint16_t topLeftX = reg.x < touchBorder ? 0 : reg.x - touchBorder;
        uint16_t topLeftY = reg.y < touchBorder ? 0 : reg.y - touchBorder;

        uint16_t bottomRightX = reg.x + reg.w + touchBorder;
        uint16_t bottomRightY = reg.y + reg.h + touchBorder;

        DBG("TouchRegion: TL: (%d, %d), BR: (%d, %d) CMD: %d", topLeftX, topLeftY, bottomRightX, bottomRightY, reg.cmd);

        if (tp.xPos > topLeftX && tp.xPos < bottomRightX && tp.yPos > topLeftY && tp.yPos < bottomRightY)
        {
            DBG(" >> Match\n");
            return reg.cmd;
        }
        else
        {
            DBG(" >> NOP\n");
        }
    }
    return TouchCmd::TOUCH_NONE;
}