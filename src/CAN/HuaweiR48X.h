#pragma once

#include <Arduino.h>
#include <defines.h>
#include <memory>
#include <CAN/CAN.h>
#include <CAN/CANPSU.h>
#include <CAN/ICANHandler.h>

#define MAX_CURRENT_MULTIPLIER 20

#define R48xx_DATA_INPUT_POWER 0x70
#define R48xx_DATA_INPUT_FREQ 0x71
#define R48xx_DATA_INPUT_CURRENT 0x72
#define R48xx_DATA_OUTPUT_POWER 0x73
#define R48xx_DATA_EFFICIENCY 0x74
#define R48xx_DATA_OUTPUT_VOLTAGE 0x75
#define R48xx_DATA_OUTPUT_CURRENT_MAX 0x76
#define R48xx_DATA_INPUT_VOLTAGE 0x78
#define R48xx_DATA_OUTPUT_TEMPERATURE 0x7F
#define R48xx_DATA_INPUT_TEMPERATURE 0x80
#define R48xx_DATA_OUTPUT_CURRENT 0x81
#define R48xx_DATA_OUTPUT_CURRENT1 0x82
/*
ID: 0x1081407f Length: 8 Data: 0x01 0x0E 0x00 0x00 0x00 0x00 0x00 0x01
R48xx_DATA_INPUT_POWER
ID: 0x1081407f Length: 8 Data: 0x01 0x70 0x00 0x00 0x00 0x00 0x00 0x00
R48xx_DATA_INPUT_FREQ
ID: 0x1081407f Length: 8 Data: 0x01 0x71 0x00 0x00 0x00 0x00 0xF0 0x14
R48xx_DATA_INPUT_CURRENT
ID: 0x1081407f Length: 8 Data: 0x01 0x72 0x00 0x00 0x00 0x00 0x00 0x00
R48xx_DATA_OUTPUT_POWER
ID: 0x1081407f Length: 8 Data: 0x01 0x73 0x00 0x00 0x00 0x00 0x00 0x00
R48xx_DATA_EFFICIENCY
ID: 0x1081407f Length: 8 Data: 0x01 0x74 0x00 0x00 0x00 0x00 0x00 0x00
R48xx_DATA_OUTPUT_VOLTAGE
ID: 0x1081407f Length: 8 Data: 0x01 0x75 0x00 0x00 0x00 0x00 0xB7 0xD7
R48xx_DATA_OUTPUT_CURRENT_MAX
ID: 0x1081407f Length: 8 Data: 0x01 0x76 0x00 0x00 0x00 0x00 0x03 0x88
R48xx_DATA_OUTPUT_CURRENT
ID: 0x1081407f Length: 8 Data: 0x01 0x81 0x00 0x00 0x00 0x00 0x00 0x00

*/

#define HUAWEI_MAX_CURRENT_220 56
#define HUAWEI_MAX_CURRENT_110 25

class HuaweiR48X : public ICANHandler
{

public:
    ~HuaweiR48X();
    void begin(CAN *can) override;
    void end() override;
    void onStatusCallback(PowerSupplyStatusCb cb) override;
    void onAlarmCallback(PowerSupplyAlarmCb cb) override;
    void setStartupVoltage(float volts) override;
    void setOperationalVoltageAndCurrent(float volts, float amps) override;
    float maxCurrent() override;
    void loop() override;
    void processMessage(uint32_t id, uint8_t len, uint8_t data[8]) override;

private:
    void requestData();
    void handleDataFrame(uint8_t *frame);
    void handleAck(uint8_t *frame);
    void handleDescription(uint8_t *frame);
    void setOperationalVoltage(float volts);
    void setOperationalCurrent(float amps);

    CAN *_can;
    PowerSupplyStatusCb _statusCb;
    PowerSupplyAlarmCb _alarmCb;
    bool _started = false;
    time_t _nextPool;
    time_t _lastUpdate;
    time_t _lastDebug;
    time_t _nextVoltSample;
    uint16_t _poolingInterval = 200;

    PowerSupplyMode _mode = PowerSupplyMode::POWEROFF;
    int _intakeTemperature;
    float _current;
    float _outputVoltage;
    int _inputVoltage;
    int _outputTemperature;
    float _maxCurrent;
};
