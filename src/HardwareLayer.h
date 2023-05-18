#include <Arduino.h>
#include <memory>
#include <functional>
#include <Defines.h>
#include <Types.h>
#include <Button.h>
#include <LatchingRelay.h>
#include <TFT_eSPI.h>
#include <FT62XXTouchScreen.h>
#include <ADS1X15.h>
#include <DS18B20.h>
#include <OneWire.h>
#include <ErriezMCP23017.h>

class HardwareLayer
{
    typedef std::function<void(float forwardW, float reverseW)> RfPowerSwrCb;
    typedef std::function<void(float forwardW)> RfPowerCb;
    typedef std::function<void(int button, bool longPress)> ButtonPressedCb;
    typedef std::function<void(Diag diag)> DiagCb;
    typedef std::function<void(float temperature)> TemperatureCb;
    typedef std::function<void(TouchPoint tp)> TouchCb;

public:
    HardwareLayer();

    void onOutputRfPowerCallback(RfPowerSwrCb cb);
    void onInputRfPowerCallback(RfPowerCb cb);
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
    static void task(void *pvParameters);
    void setFanSpeed(uint8_t duty);
    void setBacklightLevel(uint8_t duty);
    
private:
    void setAmplifier(Amplifier amp);
    void setLowPassFilter(LowPassFilter lpf);
    float readTemperature();
    float readVoltage(uint8_t pin);
    float readVoltageSamples(uint8_t pin, uint8_t samples);

    RfPowerSwrCb _outputPowerCallback;
    RfPowerCb _inputPowerCallback;
    DiagCb _diagCallback;
    ButtonPressedCb _buttonPressed;
    TemperatureCb _temperatureCallback;
    TouchCb _touchCallback;

    bool _txOn = false;
    bool _started = false;
    
    void readOutputPower();
    void readInputPower();

    float readRfPower(ADS1115 *adc, float factor);

    bool deviceReady(uint8_t address);
    void doHigh();
    void doLow();
    void doToggle();

    volatile time_t _timer1;
    volatile time_t _timer2;
    volatile time_t _timerPowerReading;
    volatile bool state;

    LatchingRelay _vhfRelay;
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
    float _lastOutputPower;

    ErriezMCP23017 _io = ErriezMCP23017(ADDRESS_IO_EXPANDER);
    ADS1115 _adsOutputFwd = ADS1115(ADDRESS_ADC_OUTPUT_FWD);
    ADS1115 _adsOutputRev = ADS1115(ADDRESS_ADC_OUTPUT_REV);
    ADS1115 _adsInputFwd = ADS1115(ADDRESS_ADC_INPUT_FWD);

    OneWire _oneWire = OneWire(PIN_TEMPERATURE);
    DS18B20 _temperatureSensor = DS18B20(&_oneWire);
};