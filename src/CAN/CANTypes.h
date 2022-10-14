#pragma once

#include <Arduino.h>
#include <memory>

enum PowerSupplyMode
{
    NOT_STARTED,
    STARTED,
    POWEROFF,
    NORMAL,
    RAMPING,
    WARNING,
    ALARM
};

struct PowerSupplyMessage
{
    uint32_t id;
    uint8_t len;
    uint8_t data[8];
};

typedef std::function<void(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage)> PowerSupplyStatusCb;
typedef std::function<void(uint8_t[6])> PowerSupplyAlarmCb;
