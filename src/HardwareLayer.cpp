#include "HardwareLayer.h"

HardwareLayer::HardwareLayer()
{
}

void HardwareLayer::onRfPowerCallback(RfPowerCb cb)
{
    _rfPowerCallback = cb;
}

void HardwareLayer::onDiagnosticsCallback(DiagCb cb)
{
    _diagCallback = cb;
}

void HardwareLayer::onTemperatureCallback(TemperatureCb cb)
{
    _temperatureCallback = cb;
}

void HardwareLayer::onTouchCallback(TouchCb cb)
{
    _touchCallback = cb;
}

Diag HardwareLayer::begin()
{
    Wire.begin(PIN_SDA, PIN_SCL, (uint32_t)100E3);

    if (!_io.begin(true)) //
    {
        DBG("MCP23017: Fail (IO Expander) [Core %d]\n", xPortGetCoreID());
    }
    else
    {
        DBG("MCP23017: OK (IO expander) [Core %d]\n", xPortGetCoreID());
        _diag.mainExpander = true;
        _io.setPortDirection(0xFFFF);
    }

    if (!_adsOutputFwd.isConnected()) //
    {
        DBG("ADS1X15: Fail (FWD RF Sensor) [Core %d]\n", xPortGetCoreID());
    }
    else
    {
        _adsOutputFwd.begin();
        DBG("ADS1X15: OK (FWD RF Sensor) [Core %d]\n", xPortGetCoreID());
        _diag.rfAdcFwd = true;
        _adsOutputFwd.setGain(4);
        _adsOutputFwd.setDataRate(7);
        _adsOutputFwd.setMode(0);
    }

    if (!_adsOutputRev.isConnected()) //
    {
        DBG("ADS1X15: Fail (REV RF Sensor) [Core %d]\n", xPortGetCoreID());
    }
    else
    {
        _adsOutputRev.begin();
        DBG("ADS1X15: OK (REV RF Sensor) [Core %d]\n", xPortGetCoreID());
        _diag.rfAdcRev = true;
        _adsOutputRev.setGain(8);
        _adsOutputRev.setDataRate(7);
        _adsOutputRev.setMode(0);
    }

    if (!_adsInputFwd.isConnected()) //
    {
        DBG("ADS1X15: Fail (INPUT RF Sensor) [Core %d]\n", xPortGetCoreID());
    }
    else
    {
        _adsInputFwd.begin();
        DBG("ADS1X15: OK (INPUT RF Sensor) [Core %d]\n", xPortGetCoreID());
        _diag.rfAdcInput = true;
        _adsInputFwd.setGain(8);
        _adsInputFwd.setDataRate(7);
        _adsInputFwd.setMode(0);
        _adsInputFwd.requestADC(ADC_RF_CHANNEL_INPUT_FWD);
    }

    bool sensorFound = _temperatureSensor.begin();

    if (sensorFound)
    {
        _temperatureSensor.setResolution(11);
        _temperatureSensor.requestTemperatures();
        uint8_t count = 0;
        while (count < 50 && !_temperatureSensor.isConversionComplete())
        {
            count++;
            delay(100);
        }
        _diag.temperature = _temperatureSensor.isConversionComplete();
        if (_diag.temperature)
        {
            _lastTemperature = _temperatureSensor.getTempC();
            DBG("DS18B20: Temperature: %.2f\n", _lastTemperature);
            _diag.temperatureCelsius = _lastTemperature;
        }
        else
        {
            DBG("DS18B20: Timeout reading temperature\n");
        }
    }
    else
    {
        DBG("DS18B20: Sensor not found\n");
    }

    if (_diagCallback)
    {
        _diagCallback(_diag);
    }

#ifdef PIN_FAN_SPEED
    pinMode(PIN_FAN_SPEED, OUTPUT);
    ledcSetup(FAN_PWM_CHANNEL, 20, 8);
    ledcAttachPin(PIN_FAN_SPEED, FAN_PWM_CHANNEL);
    setFanSpeed(0);
#endif

#ifdef TFT_BL
    ledcSetup(BLK_PWM_CHANNEL, 5000, 8);
    pinMode(TFT_BL, OUTPUT);
    ledcAttachPin(TFT_BL, BLK_PWM_CHANNEL);
    setBacklightLevel(DEFAULT_BACKLIGHT_LEVEL);
#endif

#ifdef WT32SC01
    _touch.begin();
#endif

    return _diag;
}

void HardwareLayer::end()
{
    if (_previousLpfPin < 255)
    {
        _io.digitalWrite(_previousLpfPin, LOW);
        _previousLpfPin = 255;
    }
}

void HardwareLayer::started(bool state)
{
    _started = state;
    if (!_started)
    {
        _io.portWrite(0x00);
    }
    else
    {
        if (_previousLpfPin < 255)
        {
            _io.digitalWrite(_previousLpfPin, HIGH);
        }
        _io.digitalWrite(IO_PIN_HF_VHF, _statePinHFVHF);
    }
}

void HardwareLayer::loop()
{
    if (millis() > _timer1)
    {
        if (_diag.rfAdcFwd && _diag.rfAdcRev)
        {
            readRfPower();
        }
        _timer1 = millis() + 10;
    }

    if (millis() > _timer2)
    {

        if (_temperatureCallback && _diag.temperature && _started)
        {
            float temp = readTemperature();
            if (temp > -127)
            {
                _temperatureCallback(temp);
            }
        }
        _timer2 = millis() + 1000;

        if (true)
        {
            float volts1 = 0;
            uint8_t gain1 = 0;
            float volts2 = 0;
            uint8_t gain2 = 0;
            float volts3 = 0;
            uint8_t gain3 = 0;
            if (_diag.rfAdcFwd)
            {
                volts1 = getAdcVoltage(&_adsOutputFwd, _channelFwd);
                gain1 = _adsOutputFwd.getGain();
            }
            if (_diag.rfAdcRev)
            {
                volts2 = getAdcVoltage(&_adsOutputRev, _channelRev);
                gain2 = _adsOutputRev.getGain();
            }
            if (_diag.rfAdcInput)
            {
                volts3 = getAdcVoltage(&_adsInputFwd, ADC_RF_CHANNEL_INPUT_FWD);
                gain3 = _adsInputFwd.getGain();
            }
            DBG("Voltage/Gain: Input [Fwd: %.4f@%d], Output [FWD: %.4f@%d, REV: %.4f@%d]\n", volts3, gain3, volts1, gain1, volts2, gain2);
        }
    }

#ifdef WT32SC01
    if (_diag.mainExpander)
    {
        TouchPoint touchPos = _touch.read();
        if (touchPos.touched && _touchCallback)
        {
            _touchCallback(touchPos);
            // DBG("Touch X: %d, Y: %d\n", touchPos.xPos, touchPos.yPos);
        }
    }
#endif
}

void HardwareLayer::setAmplifier(Amplifier amp)
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
        _io.digitalWrite(IO_PIN_TX_RX_VHF, LOW);
        _io.digitalWrite(IO_PIN_TX_RX_HF, LOW);
        if (amp == AMP_HF)
        {
            _channelFwd = ADC_RF_CHANNEL_HF_FWD;
            _channelRev = ADC_RF_CHANNEL_HF_REV;
            _statePinHFVHF = HIGH;
        }
        else
        {
            _channelFwd = ADC_RF_CHANNEL_VHF_FWD;
            _channelRev = ADC_RF_CHANNEL_VHF_REV;
            _statePinHFVHF = LOW;
        }
        _adsOutputFwd.requestADC(_channelFwd);
        _adsOutputRev.requestADC(_channelRev);
        _io.digitalWrite(IO_PIN_HF_VHF, _statePinHFVHF);
        DBG("Amplifier changed: %s (IO %d %s) [Core %d]\n", desc, IO_PIN_HF_VHF, _statePinHFVHF ? "HIGH" : "LOW", xPortGetCoreID());
    }
    else
    {
        DBG("Amplifier not ready: %s [Core %d]\n", desc, xPortGetCoreID());
    }
}

void HardwareLayer::onButtonPressedCallback(ButtonPressedCb cb)
{
    _buttonPressed = cb;
}

void HardwareLayer::setLowPassFilter(LowPassFilter lpf)
{
    bool knownLpf = true;
    const char *desc;
    uint8_t lpfPin = 255;

    switch (lpf)
    {
    case BAND_160M:
        desc = "160M";
        lpfPin = 255;
        break;
    case BAND_80M:
        desc = "80M";
        lpfPin = IO_PA5;
        break;
    case BAND_60_40M:
        desc = "60/40M";
        lpfPin = IO_PA4;
        break;
    case BAND_30_20M:
        desc = "30/20M";
        lpfPin = IO_PA3;
        break;
    case BAND_17_15M:
        desc = "17/15M";
        lpfPin = IO_PA2;
        break;
    case BAND_12_10M:
        desc = "12/10M";
        lpfPin = IO_PA1;
        break;
    case BAND_6M:
        desc = "6M";
        lpfPin = IO_PA0;
        break;
    default:
        desc = "Other";
        knownLpf = false;
        break;
    }

    _lpf = lpf;
    if (knownLpf)
    {
        if (_previousLpfPin < 255)
        {
            _io.digitalWrite(_previousLpfPin, LOW);
        }

        DBG("LPF changed: %s (IO PIN %d) [Core %d]\n", desc, lpfPin, xPortGetCoreID());
        if (lpfPin < 255)
        {
            _io.digitalWrite(lpfPin, HIGH);
        }
        _previousLpfPin = lpfPin;
    }
    else
    {
        DBG("Unknown LPF: %s\n", desc);
    }
}

void HardwareLayer::onBandChanged(uint32_t freq, Band band)
{
    _band = band;
    Amplifier amp = (freq > FREQ_VHF_AMP) ? AMP_VHF : AMP_HF;

    _outputPowerFactorFwd = band.outputPowerFactorFwd;
    _outputPowerFactorRev = band.outputPowerFactorRev;
    _inputPowerFactor = band.inputPowerFactor;

    if (amp != _amp)
    {
        setAmplifier(amp);
    }
    if (amp == AMP_HF && band.lpf != _lpf)
    {
        setLowPassFilter(band.lpf);
    }
}

void HardwareLayer::onTransmitChanged(bool state)
{
    uint8_t pinRelay = 0;
    uint8_t pinBias = 0;
    if (_amp == AMP_VHF)
    {
        pinRelay = IO_PIN_TX_RX_VHF;
        pinBias = IO_PIN_BIAS_VHF;
    }
    else
    {
        pinRelay = IO_PIN_TX_RX_HF;
        pinBias = IO_PIN_BIAS_HF;
    }
    _io.digitalWrite(pinRelay, state);
    _io.digitalWrite(pinBias, state);
    _txOn = state;
    DBG("TX %s: %s (IO PIN %d) [Core %d]\n", _amp == AMP_HF ? "HF" : "VHF", state ? "ON" : "OFF", pinRelay, xPortGetCoreID());
    // DBG("BIAS %s: %s (IO PIN %d) [Core %d]\n", _amp == AMP_HF ? "HF" : "VHF", state ? "ON" : "OFF", pinBias, xPortGetCoreID());
}

bool HardwareLayer::deviceReady(uint8_t address)
{
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

void HardwareLayer::readRfPower()
{
    float inputPowerW = 0;
    if (_diag.rfAdcInput && _diag.rfAdcFwd && _diag.rfAdcRev)
    {
        float inputVolts = getAdcVoltage(&_adsInputFwd, ADC_RF_CHANNEL_INPUT_FWD);
        float fwdVolts = getAdcVoltage(&_adsOutputFwd, _channelFwd);
        float revVolts = getAdcVoltage(&_adsOutputRev, _channelRev);

        if (inputVolts > 0.2f || fwdVolts > 0.2f)
        {
            float inputPowerW = ((_diodeAdjustA * sq(inputVolts)) + (_diodeAdjustB * inputVolts)) * _inputPowerFactor;
            // float revVolts = (_adsOutputRev.readADC(_channelRev) * _adsOutputRev.toVoltage());
            float revPwr = ((_diodeAdjustA * sq(revVolts)) + (_diodeAdjustB * revVolts)) * (_outputPowerFactorRev);

            // float fwdVolts = (_adsOutputFwd.readADC(_channelFwd) * _adsOutputFwd.toVoltage());
            float fwdPwr = ((_diodeAdjustA * sq(fwdVolts)) + (_diodeAdjustB * fwdVolts)) * (_outputPowerFactorFwd);

            if (_rfPowerCallback)
                _rfPowerCallback(inputPowerW, fwdPwr, revPwr);
        }
        else
        {
            if (_rfPowerCallback)
                _rfPowerCallback(0.0f, 0.0f, 0.0f);
        }
    }
}

float HardwareLayer::getAdcVoltage(ADS1115 *ads, uint8_t channel)
{
    if (ads != nullptr)
    {
        uint8_t gain = ads->getGain();
        // float volts = (ads->readADC(ADC_RF_CHANNEL_INPUT_FWD) * ads->toVoltage());
        float volts = ads->getValue() * ads->toVoltage(1);
        if (volts >= ads->getMaxVoltage() && gain > 0)
        {
            uint8_t newGain = gain / 2;
            ads->setGain(newGain);
            ads->requestADC(channel);
            volts = ads->getValue() * ads->toVoltage(1);
        }
        return volts;
    }
    return 0.0f;
}

void HardwareLayer::onPowerSupplyChanged(bool state)
{
    _io.digitalWrite(IO_PIN_PSU_ON, state);
    DBG("PSU: Turning %s (PIN %d) [Core %d]\n", state ? "ON" : "OFF", IO_PIN_PSU_ON, xPortGetCoreID());
}

float HardwareLayer::readTemperature()
{
    if (_temperatureSensor.isConversionComplete())
    {
        _lastTemperature = _temperatureSensor.getTempC();
        _temperatureSensor.requestTemperatures();
    }
    // DBG("HW Temperature: %.2foC\n", _lastTemperature);
    return _lastTemperature;
}

float HardwareLayer::readVoltageInternalADC(uint8_t pin)
{
    return analogRead(pin) * (3.3 / 4095.0);
}

void HardwareLayer::setFanSpeed(uint8_t percent)
{
#ifdef PIN_FAN_SPEED
    if (_diag.mainExpander)
    {
        uint32_t duty = 0;
        if (percent > 0)
        {
            duty = map(percent, 0, 100, 60, 255);
        }
        // DBG("FAN: %d%%, Duty: %d\n", percent, duty);
        ledcWrite(FAN_PWM_CHANNEL, duty);
    }
#endif
}

void HardwareLayer::setBacklightLevel(uint8_t percent)
{
#ifdef TFT_BL
    uint8_t duty = (percent / 100.0f) * 255;
    DBG("Backlight: %d%%, Duty: %d\n", percent, duty);
    ledcWrite(BLK_PWM_CHANNEL, duty);
#endif
}