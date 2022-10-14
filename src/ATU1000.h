#ifndef _ATU_1000_CLASS_
#define _ATU_1000_CLASS_

#define ATU_RX_PIN 33

#include <Arduino.h>
#include "Defines.h"
#include "Types.h"
#include <memory>

// typedef std::function<void(uint8_t[6])> PowerSupplyStatusCb;
// typedef std::function<void(uint8_t[6])> PowerSupplyAlarmCb;

class ATU1000
{

public:
    ATU1000();
    void begin();
    void end();
    void loop();

private:
};
#endif
