

#pragma once

#include <Arduino.h>
#include <defines.h>
#include <memory>

#include <CAN/CAN.h>
#include <CAN/ICANHandler.h>
#include <CAN/HuaweiR48X.h>
#include <CAN/EltekFP2.h>
#include <CAN/CANTypes.h>

class CANPSU
{

public:
    CANPSU();
    void begin();
    void end();
    void onPowerSupplyStatus(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage);
    void onStatusCallback(PowerSupplyStatusCb cb);
    void onAlarmCallback(PowerSupplyAlarmCb cb);
    void setStartupVoltage(float volts);
    float maxCurrent();
    void setOperationalVoltageAndCurrent(float volts, float amps);
    void loop();
    static void printMessage(uint32_t id, uint8_t len, uint8_t *data);

private:
    CAN _can;
    PowerSupplyStatusCb _statusCb;
    PowerSupplyAlarmCb _alarmCb;
    ICANHandler *_handler;
    bool _started = false;
    bool _detected = false;
    PowerSupplyMode _mode;
    volatile time_t _lastUpdate;
    volatile time_t _lastDebug;
};
