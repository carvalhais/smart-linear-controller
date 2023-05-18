#include "Supervisor.h"

Supervisor::Supervisor()
{
}

void Supervisor::begin()
{
}

void Supervisor::onFanSpeedCallback(FanSpeedCb cb)
{
    _fanSpeedCb = cb;
}

void Supervisor::updateTemperature(float temperature)
{
    float alpha = temperature > _temperatureAccumulator ? 1 : 0.1f; // 1 = fast 0.1 = slow
    _temperatureAccumulator += alpha * (temperature - _temperatureAccumulator);

    uint8_t t = minimum(_temperatureAccumulator, _temperatureMax);
    t = maximum(t, _temperatureMin);

    uint8_t fanSpeed = map(t, _temperatureMin, _temperatureMax, 0, 100);

    if (fanSpeed != _lastFanSpeed)
    {
        _lastFanSpeed = fanSpeed;

        DBG("Supervisor: Fan speed set to %d%% (Temp: %.2foC, Avg: %.2foC) [Core %d]\n", fanSpeed, temperature, _temperatureAccumulator, xPortGetCoreID());

        if (_fanSpeedCb)
        {
            _fanSpeedCb(fanSpeed);
        }
    }
}

void Supervisor::updateCurrent(float current)
{
}

void Supervisor::updateRfPower(float inputPower, float outputPower, float outputSwr, float gain)
{
}

void Supervisor::end()
{
    _lastFanSpeed = 0;
    if (_fanSpeedCb)
    {
        _fanSpeedCb(_lastFanSpeed);
    }
}

void Supervisor::loop()
{
}
