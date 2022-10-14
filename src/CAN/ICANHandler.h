#pragma once

#include <memory>
#include <CAN/CAN.h>

class ICANHandler
{
public:
    virtual ~ICANHandler(){};
    virtual void begin(CAN *can) = 0;
    virtual void end() = 0;
    virtual void onStatusCallback(PowerSupplyStatusCb cb) = 0;
    virtual void onAlarmCallback(PowerSupplyAlarmCb cb) = 0;
    virtual void setStartupVoltage(float volts) = 0;
    virtual void setOperationalVoltageAndCurrent(float volts, float amps) = 0;
    virtual void processMessage(uint32_t id, uint8_t len, uint8_t *data) = 0;
    virtual float maxCurrent() = 0;
    virtual void loop() = 0;
};
