#include <Arduino.h>
#include <memory>
#include <functional>
#include <Defines.h>
#include <Types.h>
#include <Button.h>
#include <TFT_eSPI.h>
#include <FT62XXTouchScreen.h>
#include <ADS1X15.h>
#include <DS18B20.h>
#include <OneWire.h>
#include <ErriezMCP23017.h>

class HardwareLayer
{
    typedef std::function<void(float inputW, float forwardW, float reverseW)> RfPowerCb;
    typedef std::function<void(int button, bool longPress)> ButtonPressedCb;
    typedef std::function<void(Diag diag)> DiagCb;
    typedef std::function<void(float temperature)> TemperatureCb;
    typedef std::function<void(TouchPoint tp)> TouchCb;

public:
    HardwareLayer();

    void onRfPowerCallback(RfPowerCb cb);
    void onDiagnosticsCallback(DiagCb cb);
    void onTemperatureCallback(TemperatureCb cb);
    void onTouchCallback(TouchCb cb);
    void onBandChanged(uint32_t freq, Band band);
    void onTransmitChanged(bool state);
    void onPowerSupplyChanged(bool state);
    void onButtonPressedCallback(ButtonPressedCb cb);

    Diag begin();
    void end();
    void loop();
    void started(bool state);
    void setFanSpeed(uint8_t duty);
    void setBacklightLevel(uint8_t duty);

private:
    void setAmplifier(Amplifier amp);
    void setLowPassFilter(LowPassFilter lpf);
    float readTemperature();
    float readVoltageInternalADC(uint8_t pin);

    RfPowerCb _rfPowerCallback;
    DiagCb _diagCallback;
    ButtonPressedCb _buttonPressed;
    TemperatureCb _temperatureCallback;
    TouchCb _touchCallback;

    bool _txOn = false;
    bool _started = false;

    void readRfPower();
    bool deviceReady(uint8_t address);
    float getAdcVoltage(ADS1115 *ads, uint8_t channel);

    volatile time_t _timer1;
    volatile time_t _timer2;
    volatile bool state;

    LowPassFilter _lpf = BAND_OTHER;
    uint8_t _previousLpfPin = 255;
    uint8_t _statePinHFVHF = LOW;
    Amplifier _amp = AMP_UNKNOWN;
    float _lastTemperature = 0;
    Band _band;
    Button _buttonA;
    Button _buttonB;
    Button _buttonC;

    Diag _diag;

    FT62XXTouchScreen _touch = FT62XXTouchScreen(TFT_WIDTH, PIN_SDA, PIN_SCL);

    float _outputPowerFactorFwd;
    float _outputPowerFactorRev;
    float _inputPowerFactor;

    uint8_t _channelFwd;
    uint8_t _channelRev;

    ErriezMCP23017 _io = ErriezMCP23017(ADDRESS_IO_EXPANDER);
    ADS1115 _adsOutputFwd = ADS1115(ADDRESS_ADC_OUTPUT_FWD);
    ADS1115 _adsOutputRev = ADS1115(ADDRESS_ADC_OUTPUT_REV);
    ADS1115 _adsInputFwd = ADS1115(ADDRESS_ADC_INPUT_FWD);

    OneWire _oneWire = OneWire(PIN_TEMPERATURE);
    DS18B20 _temperatureSensor = DS18B20(&_oneWire);

    float _diodeAdjustA = 1.0;
    float _diodeAdjustB = 0.75f;
};