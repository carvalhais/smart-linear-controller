#pragma once

#include <Arduino.h>
#include <defines.h>
#include <memory>
#include <CAN/CAN.h>
#include <CAN/ICANHandler.h>

#pragma region Docs
/*
Within 5s of the "login" you then send your "request" with ID: 0x05FF4004.
Current in dA (Ampere times 10) in HEX with low byte first. For 16.0A translate 160 to HEX => 0x00A0, separate into 2 bytes and place as byte 0 & 1.
Voltage in cV (Voltage times 100) in HEX with low byte first. For 57.6V translate 5760 to HEX => 0x1680, separate into 2 bytes and place as byte 2 & 3 and also byte 4 & 5.
An over voltage protection level also needs to be set, above the requested voltage. Done in same manner as Voltage so for 59.5V translate 5950 to HEX => 0x173E, separate into 2 bytes and place as byte 6 & 7.

unsigned char stmp2[8] = {0xA0, 0x00, 0x80, 0x16, 0x80, 0x16, 0x3E, 0x17};    //set rectifier to 16.0 amps (00 A0) 57.60 (16 80) and OVP to 59.5V (17 3E)
CAN.sendMsgBuf(0x05FF4004, 1, 8, stmp2);

This default voltage can be set by sending first login and then a message with ID: 0x05009C00.
Byte 0 = 0x29, Byte 1 = 0x15, Byte 2= 0x00.
For 43.7V translate 4370 to HEX => 0x1112, separate into 2 bytes and place as byte 3 & 4.


byte stmp2[5] = {0x29, 0x15, 0x00, 0x12, 0x11};                   //set rectifier permanently to 43.70 (11 12)
CAN.sendMsgBuf(0x05009C00, 1, 5, stmp2);

Voltage can be set between 43.5V and 57.5V. Current can be set between 0 and max 62.5A for a 3000W FP.
NOTE: When using current limiting the FP will only maintain set current if it can do so with a voltage above 47.0V. If voltage needs to reduce further the current output will be max current, 62.5A so be careful when designing the charging algorithm.

*/
/*


where each bit of YY relates to the following WARNINGS/ALARMS
80 = Current limit
40 = low temp
20 = High temp
10 = Low Mains
08 = High Mains
04 = Mod Fail Secondary
02 = Mod Fail Primary
01 = OVS Lock Out
and each bit of the of ZZ relates to the following WARNINGS/ALARMS
80 = Inner Volt
40 = Fan3 Speed low
20 = Sub Mod1 fail
10 = Fan2 Speed low
08 = Fan1 Speed low
04 = Mod Fail Secondary
02 = Module fail
01 = Internal Voltage


*/
#pragma endregion

#define ELTEK_MAX_CURRENT_220 56
#define ELTEK_MAX_CURRENT_110 25

class EltekFP2 : public ICANHandler
{

public:
    ~EltekFP2();
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
    void logIn();
    void storeSerialNumber(uint32_t rxID, uint8_t len, uint8_t rxBuf[]);
    void processStatusMessage(uint32_t rxID, uint8_t len, uint8_t rxBuf[]);
    void processWarningOrAlarmMessage(uint32_t rxID, uint8_t len, uint8_t rxBuf[]);
    CAN *_can;
    PowerSupplyStatusCb _statusCb;
    PowerSupplyAlarmCb _alarmCb;
    const char *_alarms0Strings[8] = {"OVS_LOCK_OUT", "MOD_FAIL_PRIMARY", "MOD_FAIL_SECONDARY", "HIGH_MAINS", "LOW_MAINS", "HIGH_TEMP", "LOW_TEMP", "CURRENT_LIMIT"};
    const char *_alarms1Strings[8] = {"INTERNAL_VOLTAGE", "MODULE_FAIL", "MOD_FAIL_SECONDARY", "FAN1_SPEED_LOW", "FAN2_SPEED_LOW", "SUB_MOD1_FAIL", "FAN3_SPEED_LOW", "INNER_VOLT"};
    bool _serialNumberReceived = false;
    uint8_t _serialNumber[8];
    bool _started = false;
    PowerSupplyMode _mode = PowerSupplyMode::POWEROFF;
    int _intakeTemperature;
    float _current;
    float _outputVoltage;
    int _inputVoltage;
    int _outputTemperature;

    volatile time_t _lastUpdate;
    volatile time_t _lastDebug;
};
