#include <Arduino.h>
#include <memory>
#include <functional>
#include <Defines.h>
#include <Types.h>
#include <Button.h>
#include <LatchingRelay.h>
#include <PCF8575.h>
#include <TFT_eSPI.h>
#include <FT62XXTouchScreen.h>
#include <ADS1X15.h>

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
    static void task(void *pvParameters);
    void setFanSpeed(uint8_t duty);
    void setBacklightLevel(uint8_t duty);

private:
    void setAmplifier(Amplifier amp);
    void setLowPassFilter(LowPassFilter lpf);
    float readTemperature();

    RfPowerSwrCb _outputPowerCallback;
    RfPowerCb _inputPowerCallback;
    DiagCb _diagCallback;
    ButtonPressedCb _buttonPressed;
    TemperatureCb _temperatureCallback;
    TouchCb _touchCallback;

    void readOutputPower();
    void readInputPower();

    float readRfPower(uint8_t index, float intercept);

    bool deviceReady(uint8_t address);
    void doHigh();
    void doLow();
    void doToggle();

    volatile time_t _timer1;
    volatile time_t _timer2;

    LatchingRelay _vhfRelay;
    LowPassFilter _lpf = BAND_OTHER;
    uint8_t _previousLpfPin = 255;
    Amplifier _amp = AMP_UNKNOWN;
    uint16_t _lastTemperature = 0;

    bool _useLm35 = false;

    float _supplyVoltage = 5.5;
    uint8_t _ntcResistance = 10; // K Ohms
    float _ntcResistanceTemperature = 25 + 273.15;
    uint8_t _ntcSeriesResistor = 10; // K Ohms
    uint16_t _ntcBetaConstant = 3950;

    float _lm35Constant = 10.0f / 1000;

    Band _band;
    Button _buttonA;
    Button _buttonB;
    Button _buttonC;

    Diag _diag;

    FT62XXTouchScreen _touch = FT62XXTouchScreen(TFT_WIDTH, PIN_SDA, PIN_SCL);

    uint8_t _adcIndexRfPowerForward;
    uint8_t _adcIndexRfPowerReverse;
    float _interceptFwd;
    float _interceptRev;
    float _inputPowerFactor;

    PCF8575 _pcf = PCF8575(ADDRESS_IO_EXPANDER);
    ADS1115 _adsOutput = ADS1115(ADDRESS_ADC_OUTPUT_1);
    ADS1115 _adsMain = ADS1115(ADDRESS_ADC_MAIN_BOARD);
};