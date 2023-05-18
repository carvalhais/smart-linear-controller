#include "ScreenBase.h"

ScreenBase::ScreenBase()
{
}

TouchCmd ScreenBase::getTouchRegion(TouchPoint tp)
{
    for (uint8_t i = 0; i < _numTouchRegions; i++)
    {
        TouchRegion reg = _touchRegions[i];

        uint16_t topLeftX = reg.xPos();
        uint16_t topLeftY = reg.yPos();

        uint16_t bottomRightX = reg.xPos() + reg.width();
        uint16_t bottomRightY = reg.yPos() + reg.height();

        DBG("TouchRegion: TL: (%d, %d), BR: (%d, %d) CMD: %d", topLeftX, topLeftY, bottomRightX, bottomRightY, reg.command());

        if (tp.xPos > topLeftX && tp.xPos < bottomRightX && tp.yPos > topLeftY && tp.yPos < bottomRightY)
        {
            DBG(" >> Match\n");
            return reg.command();
        }
        else
        {
            DBG(" >> NOP\n");
        }
    }
    return TouchCmd::TOUCH_NONE;
}