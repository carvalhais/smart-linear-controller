#pragma once

#include <Arduino.h>
#include <memory>
#include <Types.h>
#include <Defines.h>

class Supervisor
{
    typedef std::function<void(uint8_t perc)> FanSpeedCb;

public:
    Supervisor();
    void begin();
    void loop();
    void end();

    void updateTemperature(float temperature);
    void updateCurrent(float current);
    void updateRfPower(float inputPower, float outputPower, float outputSwr, float gain);
    void onFanSpeedCallback(FanSpeedCb cb);

private:
    FanSpeedCb _fanSpeedCb;
    float _temperatureAccumulator;
    uint8_t _lastFanSpeed = 0;
    uint8_t _temperatureMin = 35;
    uint8_t _temperatureMax = 50;
};
