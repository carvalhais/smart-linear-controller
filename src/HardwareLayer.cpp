#include "HardwareLayer.h"

HardwareLayer::HardwareLayer()
{
}

void HardwareLayer::onOutputRfPowerCallback(RfPowerSwrCb cb)
{
    _outputPowerCallback = cb;
}

void HardwareLayer::onInputRfPowerCallback(RfPowerCb cb)
{
    _inputPowerCallback = cb;
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
    Wire.begin(PIN_SDA, PIN_SCL, (uint32_t)400E3);

    if (!_io.isConnected()) //
    {
        DBG("MCP23017: Fail (IO Expander) [Core %d]\n", xPortGetCoreID());
    }
    else
    {
        //_io = ioFrom23017(ADDRESS_IO_EXPANDER);
        DBG("MCP23017: OK (IO expander) [Core %d]\n", xPortGetCoreID());
        _diag.mainExpander = true;
        _vhfRelay.begin(&_io, IO_PIN_PULSE_TX, IO_PIN_PULSE_RX);
        _io.pinMode16(0x00);
    }

    // float inputFwdVoltage = readVoltage(PIN_INPUT_FWD);

    // if (inputFwdVoltage < 1.0) //
    // {
    //     DBG("Input FW: Fail (Main Board) (%.2f) [Core %d]\n", inputFwdVoltage, xPortGetCoreID());
    // }
    // else
    // {
    //     DBG("Input FW: OK (Main Board) (%.2f) [Core %d]\n", inputFwdVoltage, xPortGetCoreID());
    //     _diag.inputBoard = true;
    // }

    if (!_adsOutputFwd.isConnected()) //
    {
        DBG("ADS1X15: Fail (FWD RF Sensor) [Core %d]\n", xPortGetCoreID());
    }
    else
    {
        _adsOutputFwd.begin();
        DBG("ADS1X15: OK (FWD RF Sensor) [Core %d]\n", xPortGetCoreID());
        _diag.rfAdcFwd = true;
        _adsOutputFwd.setGain(1);
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
        _adsOutputRev.setGain(1);
        _adsOutputRev.setDataRate(7);
        _adsOutputRev.setMode(0);
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

    // pinMode(PIN_STANDBY, OUTPUT);

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

void HardwareLayer::loop()
{
    if (millis() > _timer1)
    {
        if (_diag.rfAdcFwd && _diag.rfAdcRev)
        {
            readInputPower();
            // timer_t start = millis();
            readOutputPower();
        }
        _timer1 = millis() + 10;
    }

    if (millis() > _timer2)
    {
        if (_temperatureCallback && _diag.temperature)
        {
            float temp = readTemperature();
            if (temp > -127)
            {
                _temperatureCallback(temp);
            }
        }
        _timer2 = millis() + 1000;

        // float volts1 = _adsOutputFwd.getValue() * _adsOutputFwd.toVoltage(1);
        // float volts2 = _adsOutputRev.getValue() * _adsOutputRev.toVoltage(1);
        // DBG("FWD: %.4f, REV: %.4f\n", volts1, volts2);

        // float volts = readVoltageSamples(PIN_INPUT_FWD, 20);
        // DBG("Input Volts: %.4f\n", volts);
    }

    _vhfRelay.loop();

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
            _adsOutputFwd.requestADC(ADC_RF_CHANNEL_HF_FWD);
            _adsOutputRev.requestADC(ADC_RF_CHANNEL_HF_REV);
            _io.digitalWrite(IO_PIN_HF_VHF, HIGH);
        }
        else
        {
            _adsOutputFwd.requestADC(ADC_RF_CHANNEL_VHF_FWD);
            _adsOutputRev.requestADC(ADC_RF_CHANNEL_VHF_REV);
            _io.digitalWrite(IO_PIN_HF_VHF, LOW);
        }
        DBG("Amplifier changed: %s (IO %d %s) [Core %d]\n", desc, IO_PIN_HF_VHF, amp == AMP_HF ? "HIGH" : "LOW", xPortGetCoreID());
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

    _interceptFwd = band.interceptFwd;
    _interceptRev = band.interceptRev;
    _inputInterceptFwd = band.inputInterceptFwd;

    DBG("ADC Intercept points: FWD: %.2f, REV: %.2f\n", _interceptFwd, _interceptRev);

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
        _vhfRelay.toogle(state);
    }
    else
    {
        pinRelay = IO_PIN_TX_RX_HF;
        pinBias = IO_PIN_BIAS_HF;
    }
    _io.digitalWrite(pinRelay, state);
    _io.digitalWrite(pinBias, state);
    DBG("TX %s: %s (IO PIN %d) [Core %d]\n", _amp == AMP_HF ? "HF" : "VHF", state ? "ON" : "OFF", pinRelay, xPortGetCoreID());
    // DBG("BIAS %s: %s (IO PIN %d) [Core %d]\n", _amp == AMP_HF ? "HF" : "VHF", state ? "ON" : "OFF", pinBias, xPortGetCoreID());
}

bool HardwareLayer::deviceReady(uint8_t address)
{
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

float HardwareLayer::readRfPower(bool forward, float intercept)
{
    uint16_t raw = forward ? _adsOutputFwd.getValue() : _adsOutputRev.getValue();
    //_adsOutput.requestADC(index); // request on pin 0
    // uint16_t raw = _adsOutput.getValue();

    if (raw > 1000)
    {
        float volts = raw * (forward ? _adsOutputFwd.toVoltage(1) : _adsOutputRev.toVoltage(1)); // Convert the ADC result to volts in vout
        float powerdB = (ADC_SLOPE * volts) - intercept;                                         // convert the voltage to dBm in 50 ohms
        float powerW = pow(10.0, (powerdB - 30) / 10.0);                                         // convert dBm to watts
        // DBG("PWR: %s, V: %.4f, W: %.1f\n", forward ? "F" : "R", volts, powerW);
        return powerW;
    }
    else
    {
        return 0.0f;
    }
}

void HardwareLayer::readOutputPower()
{
    // DBG("readOutputPower\n");
    float fwdPwr = readRfPower(true, _interceptFwd);

    if (fwdPwr > .1f)
    {
        float revPwr = readRfPower(false, _interceptRev);
        // fwdPwr -= revPwr;
        //  DBG("PWR FWD %.2f REF %.2f\n", fwdPwr, revPwr);
        if (_outputPowerCallback)
            _outputPowerCallback(fwdPwr, revPwr);
    }
    else
    {
        if (_outputPowerCallback)
            _outputPowerCallback(0.0f, 0.0f);
    }
}

void HardwareLayer::readInputPower()
{
    float volts = readVoltageSamples(PIN_INPUT_FWD, 20);
    if (volts > 1.5)
    {
        float powerdB = (ADC_SLOPE * volts) - _inputInterceptFwd; // convert the voltage to dBm in 50 ohms
        float powerW = pow(10.0, (powerdB - 30) / 10.0);          // convert dBm to watts
        DBG("Input Volts %.4f, Watts: %.2f\n", volts, powerW);
        //  float powerW = pow((volts / _inputInterceptFwd), 2);
        if (_inputPowerCallback)
        {
            _inputPowerCallback(powerW);
        }
    }
    else
    {
        if (_inputPowerCallback)
        {
            _inputPowerCallback(0.0f);
        }
    }
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

float HardwareLayer::readVoltage(uint8_t pin)
{
    return analogRead(pin) * (3.3 / 4095.0);
}

float HardwareLayer::readVoltageSamples(uint8_t pin, uint8_t samples)
{
    uint16_t raw;
    for (uint8_t i = 0; i < samples; i++)
    {
        raw = max(raw, analogRead(pin));
    }
    return raw * (3.3 / 4095.0);
}

void HardwareLayer::setFanSpeed(uint8_t percent)
{
#ifdef PIN_FAN_SPEED
    if (_diag.mainExpander)
    {
        uint32_t duty = 0;
        if (percent > 0)
        {
            duty = map(percent, 0, 100, 50, 255);
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