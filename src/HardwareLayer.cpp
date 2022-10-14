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

    if (!deviceReady(ADDRESS_IO_EXPANDER)) //
    {
        DBG("PCF8575: Fail (IO Expander) [Core %d]\n", xPortGetCoreID());
    }
    else
    {
        _pcf.begin(PIN_SDA, PIN_SCL);
        DBG("PCF8575: OK (IO expander) [Core %d]\n", xPortGetCoreID());
        _diag.mainExpander = true;
        _vhfRelay.begin(&_pcf, IO_PIN_PULSE_TX, IO_PIN_PULSE_RX);
        _pcf.selectNone();
        //_pcf.write(IO_PIN_FAN, HIGH);
        pinMode(PIN_STANDBY, OUTPUT);
        digitalWrite(PIN_STANDBY, HIGH);
    }

    if (!deviceReady(ADDRESS_ADC_MAIN_BOARD)) //
    {
        DBG("ADS1X15: Fail (Main Board) [Core %d]\n", xPortGetCoreID());
    }
    else
    {
        _adsMain.begin();
        DBG("ADS1X15: OK (Main Board) [Core %d]\n", xPortGetCoreID());
        _diag.mainAdc = true;
        _adsMain.setGain(1);
        _adsMain.setDataRate(7);
        float temperature = readTemperature();
        _diag.temperature = !isnan(temperature);
        if (_diag.temperature)
        {
            DBG("NTC: Temperature: %.2f\n", temperature);
            _diag.temperatureCelsius = temperature;
        }
        else
        {
            DBG("NTC: Sensor not found\n");
        }
    }
    if (!deviceReady(ADDRESS_ADC_OUTPUT_1)) //
    {
        DBG("ADS1X15: Fail (RF Sensor) [Core %d]\n", xPortGetCoreID());
    }
    else
    {
        _adsOutput.begin();
        DBG("ADS1X15: OK (RF Sensor) [Core %d]\n", xPortGetCoreID());
        _diag.rfAdc = true;
        _adsOutput.setGain(1);
        _adsOutput.setDataRate(7);
        //_adsOutput.setMode(0); // continuous
    }

    if (_diagCallback)
    {
        _diagCallback(_diag);
    }

    pinMode(PIN_STANDBY, OUTPUT);

#ifdef PIN_FAN_SPEED
    pinMode(PIN_FAN_SPEED, OUTPUT);
    ledcSetup(FAN_PWM_CHANNEL, 25000, 8);
    ledcAttachPin(PIN_FAN_SPEED, FAN_PWM_CHANNEL);
    setFanSpeed(0);
#endif

#ifdef TFT_BL
    ledcSetup(BLK_PWM_CHANNEL, 5000, 8);
    pinMode(TFT_BL, OUTPUT);
    ledcAttachPin(TFT_BL, BLK_PWM_CHANNEL);
    setBacklightLevel(40);
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
        _pcf.write(_previousLpfPin, LOW);
        _previousLpfPin = 255;
    }
}

void HardwareLayer::loop()
{
    if (millis() > _timer1)
    {
        if (_diag.mainAdc)
        {
            // DBG("DBG: %.2fms\n", millis() - start);
            readInputPower();
        }
        if (_diag.rfAdc)
        {
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
            if (temp != _lastTemperature)
            {
                _temperatureCallback(temp);
                _lastTemperature = temp;
            }
        }
        _timer2 = millis() + 1000;
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
        _pcf.write(IO_PIN_TX_RX_VHF, LOW);
        _pcf.write(IO_PIN_TX_RX_HF, LOW);
        if (amp == AMP_HF)
        {
            _adcIndexRfPowerForward = ADC_RF_CHANNEL_HF_FWD;
            _adcIndexRfPowerReverse = ADC_RF_CHANNEL_HF_REV;
            _pcf.write(IO_PIN_HF_VHF, HIGH);
        }
        else
        {
            _adcIndexRfPowerForward = ADC_RF_CHANNEL_VHF_FWD;
            _adcIndexRfPowerReverse = ADC_RF_CHANNEL_VHF_REV;
            _pcf.write(IO_PIN_HF_VHF, LOW);
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
        lpfPin = IO_P1;
        break;
    case BAND_80M:
        desc = "80M";
        lpfPin = IO_P2;
        break;
    case BAND_60_40M:
        desc = "60/40M";
        lpfPin = IO_P3;
        break;
    case BAND_30_20M:
        desc = "30/20M";
        lpfPin = IO_P4;
        break;
    case BAND_17_15M:
        desc = "17/15M";
        lpfPin = IO_P5;
        break;
    case BAND_12_10M:
        desc = "12/10M";
        lpfPin = IO_P6;
        break;
    case BAND_6M:
        desc = "6M";
        lpfPin = IO_P7;
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
            _pcf.write(_previousLpfPin, LOW);
        }

        DBG("LPF changed: %s (IO PIN %d) [Core %d]\n", desc, lpfPin, xPortGetCoreID());
        _pcf.write(lpfPin, HIGH);
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
    _inputPowerFactor = band.inputPowerFactor;

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
    _pcf.write(pinRelay, state);
    _pcf.write(pinBias, state);
    DBG("TX %s: %s (IO PIN %d) [Core %d]\n", _amp == AMP_HF ? "HF" : "VHF", state ? "ON" : "OFF", pinRelay, xPortGetCoreID());
    // DBG("BIAS %s: %s (IO PIN %d) [Core %d]\n", _amp == AMP_HF ? "HF" : "VHF", state ? "ON" : "OFF", pinBias, xPortGetCoreID());
}

bool HardwareLayer::deviceReady(uint8_t address)
{
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

float HardwareLayer::readRfPower(uint8_t index, float intercept)
{
    uint16_t raw = _adsOutput.readADC(index);
    //_adsOutput.requestADC(index); // request on pin 0
    // uint16_t raw = _adsOutput.getValue();

    if (raw > 10000)
    {
        float volts = raw * _adsOutput.toVoltage(1);     // Convert the ADC result to volts in vout
        float powerdB = (ADC_SLOPE * volts) - intercept; // convert the voltage to dBm in 50 ohms
        float powerW = pow(10.0, (powerdB - 30) / 10.0); // convert dBm to watts
        // DBG("PWR: V: %.4f, W: %.1f\n", volts, powerW);
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
    float fwdPwr = readRfPower(_adcIndexRfPowerForward, _interceptFwd);
    if (fwdPwr > .1f)
    {
        float revPwr = readRfPower(_adcIndexRfPowerReverse, _interceptRev);
        // DBG("PWR FWD %.2f REF %.2f\n", fwdPwr, revPwr);
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
    float volts = _adsMain.readADC(ADC_MAIN_INPUT_FWD) * _adsMain.toVoltage(1);

    if (volts > 0.1)
    {
        float powerW = pow((volts / _inputPowerFactor), 2);
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
    _pcf.write(IO_PIN_PSU_ON, state);
    DBG("PSU: Turning %s (PIN %d) [Core %d]\n", state ? "ON" : "OFF", IO_PIN_PSU_ON, xPortGetCoreID());
}

float HardwareLayer::readTemperature()
{
    float volts = _adsMain.readADC(ADC_MAIN_TEMPERATURE) * _adsMain.toVoltage(1);
    // DBG("readTemperature: Volts %.2f\n", volts);

    float temperature = 0;
    if (_useLm35)
    {
        return roundf((volts / _lm35Constant) * 10) / 10;
    }
    else
    {
        if (volts > 0.3)
        { // 100oC
            // Convert voltage measured to resistance value
            // All Resistance are in kilo ohms.
            float R = (volts * _ntcSeriesResistor) / (_supplyVoltage - volts);
            /*Use R value in steinhart and hart equation
              Calculate temperature value in kelvin*/
            float T = 1 / ((1 / _ntcResistanceTemperature) + ((log(R / _ntcResistance)) / _ntcBetaConstant));
            T -= 273.15;          // Converting kelvin to celsius
            return (T * 10) / 10; // truncate to 1 decimal
        }
        else
        {
            return std::numeric_limits<float>::quiet_NaN();
        }
    }
}

void HardwareLayer::setFanSpeed(uint8_t percent)
{
#ifdef PIN_FAN_SPEED
    _pcf.write(IO_PIN_FAN, percent > 0 ? HIGH : LOW);
    if (_diag.mainExpander)
    {
        uint32_t duty = (percent / 100.0f) * 255;
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