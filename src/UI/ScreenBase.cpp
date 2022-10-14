#include "ScreenBase.h"

ScreenBase::ScreenBase()
{
}

TouchCmd ScreenBase::getTouchRegion(TouchPoint tp)
{
    for (uint8_t i = 0; i < _numTouchRegions; i++)
    {
        TouchRegion reg = _touchRegions[i];

        uint16_t bottomLeftX = reg.x;
        uint16_t bottomLeftY = reg.y + reg.h;

        uint16_t topRightX = reg.x + reg.w;
        uint16_t topRightY = reg.y;

        DBG("TouchRegion: BL: (%d, %d), TR: (%d, %d) CMD: %d", bottomLeftX, bottomLeftY, topRightX, topRightY, reg.cmd);

        if (tp.xPos > bottomLeftX && tp.yPos < bottomLeftY && tp.xPos < topRightX && tp.yPos > topRightY)
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