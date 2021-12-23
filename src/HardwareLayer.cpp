#include "HardwareLayer.h"

HardwareLayer::HardwareLayer()
{
}

void HardwareLayer::onInputSwrCallback(SwrCb cb)
{
    _inputSwrCallback = cb;
}

void HardwareLayer::onOutputSwrCallback(SwrCb cb)
{
    _outputSwrCallback = cb;
}

void HardwareLayer::onAmplifierChanged(Amplifier amp)
{
    const char *desc;
    switch (amp)
    {
    case AMP_HF:
        desc = "HF";
        break;
    case AMP_VHF:
        desc = "VHF";
        break;
    case AMP_UHF:
        desc = "UHF";
        break;
    default:
        desc = "Unknown";
        break;
    }
    _amp = amp;
    digitalWrite(PIN_HF_VHF_AMP, amp == AMP_HF ? 1 : 0);
    DBG("Amplifier changed: %s\n", desc);
}

void HardwareLayer::onLowPassFilterChanged(LowPassFilter lpf)
{

    if (lpf != _lpf && _lpfPin != 0)
    {
        digitalWrite(_lpfPin, 0);
    }

    const char *desc;
    switch (lpf)
    {
    case BAND_160M:
        desc = "160M";
        _lpfPin = PIN_LPF_160M;
        break;
    case BAND_80M:
        desc = "80M";
        _lpfPin = PIN_LPF_80M;
        break;
    case BAND_60_40M:
        desc = "60/40M";
        _lpfPin = PIN_LPF_60_40M;
        break;
    case BAND_30_20M:
        desc = "30/20M";
        _lpfPin = PIN_LPF_30_20M;
        break;
    case BAND_17_15_12M:
        desc = "17/15/12M";
        _lpfPin = PIN_LPF_17_15_12M;
        break;
    case BAND_10_11M:
        desc = "10/11M";
        _lpfPin = PIN_LPF_10_11M;
        break;
    case BAND_6M:
        desc = "6M";
        _lpfPin = PIN_LPF_6M;
        break;
    default:
        desc = "Other";
        _lpfPin = 0;
        break;
    }

    _lpf = lpf;
    if (_lpfPin > 0)
    {
        digitalWrite(_lpfPin, 1);
    }
    DBG("LPF changed: %s\n", desc);
}

void HardwareLayer::onTransmitChanged(bool state)
{
    uint8_t pin = _amp == AMP_VHF ? PIN_TX_VHF : PIN_TX_HF;
    digitalWrite(pin, state);
}

float HardwareLayer::readVpp(uint8_t pin)
{
    uint16_t reading = 0;
    for (uint8_t i = 0; i < 50; i++)
    {
        reading = max(reading, analogRead(pin));
    }
    return (reading * 3.3) / 4095.0;
}

void HardwareLayer::begin()
{
    pinMode(PIN_HF_VHF_AMP, OUTPUT);
    digitalWrite(PIN_HF_VHF_AMP, 0);

    pinMode(PIN_TX_VHF, OUTPUT);
    digitalWrite(PIN_TX_VHF, 0);

    pinMode(PIN_TX_HF, OUTPUT);
    digitalWrite(PIN_TX_HF, 0);
}

void HardwareLayer::loop()
{
    if (millis() > _timer1)
    {
        _timer1 = millis() + 100;
        _lastInputSwr = readPowerSwr(PIN_INPUT_SWR_FWD, PIN_INPUT_SWR_REV, _inputSwrCallback, _lastInputSwr);
        _lastOutputSwr = readPowerSwr(PIN_OUTPUT_SWR_FWD, PIN_OUTPUT_SWR_REV, _outputSwrCallback, _lastOutputSwr);
    }
}

float HardwareLayer::readPowerSwr(uint8_t pinForward, uint8_t pinReverse, SwrCb callback, float lastRead)
{
    float fwdMv = readVpp(pinForward);
    if (fwdMv <= 0.3) // ~1W
        fwdMv = 0;

    if (fwdMv > 0)
    {
        float revMv = readVpp(pinReverse);
        if (callback)
            callback(fwdMv, revMv);
    }
    else if (lastRead > 0.0)
    {
        if (callback)
            callback(0, 0);
    }

    return fwdMv;
}
