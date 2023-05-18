#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Types.h"
#include "Defines.h"
#include "FT62XXTouchScreen.h"
#include "TouchRegion.h"

class ScreenBase
{
public:
    ScreenBase();
    TouchCmd getTouchRegion(TouchPoint tp);

protected:
    TouchRegion *_touchRegions;
    int _numTouchRegions;
};