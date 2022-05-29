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
    bool ampReady = true;
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
        ampReady = false;
        desc = "UHF";
        break;
    default:
        ampReady = false;
        desc = "Unknown";
        break;
    }
    _amp = amp;

    if (ampReady)
    {
        digitalWrite(PIN_HF_VHF_AMP, amp == AMP_HF ? HIGH : LOW);
        DBG("Amplifier changed: %s\n", desc);
    }
    else
    {
        DBG("Amplifier not ready: %s\n", desc);
    }
}

void HardwareLayer::onButtonPressedCallback(ButtonPressedCb cb)
{
    _buttonPressed = cb;
}

void HardwareLayer::onLowPassFilterChanged(LowPassFilter lpf)
{
    bool knownLpf = true;
    const char *desc;
    uint8_t lpfA = HIGH;
    uint8_t lpfB = HIGH;
    uint8_t lpfC = HIGH;

    switch (lpf)
    {
    case BAND_160M:
        desc = "160M";
        lpfA = LOW, lpfB = LOW, lpfC = LOW;
        break;
    case BAND_80M:
        desc = "80M";
        lpfA = HIGH, lpfB = LOW, lpfC = LOW;
        break;
    case BAND_60_40M:
        desc = "60/40M";
        lpfA = LOW, lpfB = HIGH, lpfC = LOW;
        break;
    case BAND_30_20M:
        desc = "30/20M";
        lpfA = HIGH, lpfB = HIGH, lpfC = LOW;
        break;
    case BAND_17_15M:
        desc = "17/15M";
        lpfA = LOW, lpfB = LOW, lpfC = HIGH;
        break;
    case BAND_12_10M:
        desc = "12/10M";
        lpfA = HIGH, lpfB = LOW, lpfC = HIGH;
        break;
    case BAND_6M:
        desc = "6M";
        lpfA = LOW, lpfB = HIGH, lpfC = HIGH;
        break;
    default:
        desc = "Other";
        knownLpf = false;
        break;
    }

    _lpf = lpf;
    if (knownLpf)
    {
        DBG("LPF changed: %s: %s %s %s\n", desc, lpfA ? "H" : "L", lpfB ? "H" : "L", lpfC ? "H" : "L");
        digitalWrite(PIN_LPF_A, lpfA);
        digitalWrite(PIN_LPF_B, lpfB);
        digitalWrite(PIN_LPF_C, lpfC);
    }
    else
    {
        DBG("Unknown LPF: %s\n", desc);
    }
}

void HardwareLayer::onTransmitChanged(bool state)
{
    if (_amp == AMP_VHF)
    {
        digitalWrite(PIN_TX_VHF, state);
        _vhfRelay.toogle(state);
    }
    else
    {
        digitalWrite(PIN_TX_HF, state);
    }

    DBG("TX %s: %s\n", _amp == AMP_HF ? "HF" : "VHF", state ? "ON" : "OFF");
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
    pinMode(PIN_TX_VHF, OUTPUT);
    pinMode(PIN_TX_HF, OUTPUT);
    pinMode(PIN_LPF_A, OUTPUT);
    pinMode(PIN_LPF_B, OUTPUT);
    pinMode(PIN_LPF_C, OUTPUT);

    // pinMode(PIN_PSU_CONTROL, OUTPUT);
    // digitalWrite(PIN_PSU_CONTROL, 1);

    _vhfRelay.begin(PIN_PULSE_TX, PIN_PULSE_RX);

    _buttonA.begin(BUTTON_A, _buttonPressed);
    _buttonB.begin(BUTTON_B, _buttonPressed);
    _buttonC.begin(BUTTON_C, _buttonPressed);
}

void HardwareLayer::loop()
{
    if (millis() > _timer1)
    {
        _timer1 = millis() + 100;
        _lastInputSwr = readPowerSwr(PIN_INPUT_SWR_FWD, PIN_INPUT_SWR_REV, _inputSwrCallback, _lastInputSwr);
        _lastOutputSwr = readPowerSwr(PIN_OUTPUT_SWR_FWD, PIN_OUTPUT_SWR_REV, _outputSwrCallback, _lastOutputSwr);
    }

    _buttonA.loop();
    _buttonB.loop();
    _buttonC.loop();

    _vhfRelay.loop();
}

float HardwareLayer::readPowerSwr(uint8_t pinForward, uint8_t pinReverse, SwrCb callback, float lastRead)
{
    return 0;
    /*
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
    */
}

void HardwareLayer::onPowerSupplyChanged(bool state)
{
    DBG("PSU: %s\n", state ? "ON" : "OFF");
    // digitalWrite(PIN_PSU_CONTROL, state ? 0 : 1);
}
