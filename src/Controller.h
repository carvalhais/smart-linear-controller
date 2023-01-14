#include <TFT_eSPI.h>
#include <SPI.h>
#include <memory>
#include <Preferences.h>
#include <Defines.h>
#include <Types.h>
#include <ICOM.h>
#include <HardwareLayer.h>
#include <UI/UI.h>
#include <CAN/CANTypes.h>
#include <CAN/CANPSU.h>

#define BANDS_SIZE 15

class Controller
{
    typedef std::function<void(Amplifier amp)> AmplifierCb;
    typedef std::function<void(LowPassFilter lpf)> LowPassFilterCb;

public:
    Controller();
    void begin();
    void end();
    void loop();

    void onFrequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState);
    void onClientConnected(uint8_t macAddress[6]);
    void onClientDisconnected();

    void onPowerSupplyStatus(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage);
    void onMeterUpdated(uint8_t type, uint8_t value);
    void onDiagnosticsUpdated(Diag diag);
    void onTemperatureUpdated(float temperature);
    void onTouch(TouchPoint tp);

    // void onAmplifierCallback(AmplifierCb callback);
    // void onLowPassFilterCallback(LowPassFilterCb callback);

    void onOutputPower(float forwardWatts, float reverseWatts);
    void onInputPower(float forwardWatts);
    void onButtonPressed(int button, bool longPress);

private:
    void updateFrequencyWidget();
    void start();
    bool transmitEnabled();
    void setBypassState();

    float _outputPowerAccumulator = 0;
    float _inputPowerAccumulator = 0;
    float _powerGainDB = 0;
    Preferences _preferences;

    Band bandLookup(uint32_t freq);

    ICOM _rig;
    HardwareLayer _hal;
    CANPSU _psu;

    uint8_t _temperatureMin = 30;
    uint8_t _temperatureMax = 60;
    // ATU1000 _atu;
    UI _ui;

    Band HAM_BANDS[BANDS_SIZE] = {
        {
            min : 0,
            max : 0,
            name : (char *)"-",
            lpf : BAND_OTHER,
            interceptFwd : 42.2f,
            interceptRev : 37.5f,
            inputPowerFactor : 1
        },
        {
            min : 1800,
            max : 2000,
            name : (char *)"160",
            lpf : BAND_160M,
            interceptFwd : 45.5f,
            interceptRev : 40.8f, // 43.6f
            inputPowerFactor : 1.30
        },
        {
            min : 3500,
            max : 4000,
            name : (char *)"80M",
            lpf : BAND_80M,
            interceptFwd : 45.7f,
            interceptRev : 41.0f,
            inputPowerFactor : 1.28
        },
        {
            min : 6900,
            max : 6999,
            name : (char *)";-)",
            lpf : BAND_60_40M,
            interceptFwd : 45.8f,
            interceptRev : 41.1f,
            inputPowerFactor : 1.26
        },
        {
            min : 7000,
            max : 7300,
            name : (char *)"40M",
            lpf : BAND_60_40M,
            interceptFwd : 45.8f,
            interceptRev : 41.1f,
            inputPowerFactor : 1.26
        },
        {
            min : 10000,
            max : 10500,
            name : (char *)"30M",
            lpf : BAND_30_20M,
            interceptFwd : 45.7f,
            interceptRev : 41.0f,
            inputPowerFactor : 1.15
        },
        {
            min : 14000,
            max : 14350,
            name : (char *)"20M",
            lpf : BAND_30_20M,
            interceptFwd : 45.5f,
            interceptRev : 40.8f,
            inputPowerFactor : 1.05
        },
        {
            min : 18000,
            max : 18500,
            name : (char *)"17M",
            lpf : BAND_17_15M,
            interceptFwd : 45.1f,
            interceptRev : 40.4f,
            inputPowerFactor : 0.94
        },
        {
            min : 21000,
            max : 21500,
            name : (char *)"15M",
            lpf : BAND_17_15M,
            interceptFwd : 45.0f,
            interceptRev : 40.3f,
            inputPowerFactor : 0.90
        },
        {
            min : 24500,
            max : 25000,
            name : (char *)"12M",
            lpf : BAND_12_10M,
            interceptFwd : 44.9f,
            interceptRev : 40.2f,
            inputPowerFactor : 0.87
        },
        {
            min : 26965,
            max : 27855,
            name : (char *)"11M",
            lpf : BAND_12_10M,
            interceptFwd : 44.4f,
            interceptRev : 39.7f,
            inputPowerFactor : 0.72
        },
        {
            min : 28000,
            max : 29700,
            name : (char *)"10M",
            lpf : BAND_12_10M,
            interceptFwd : 44.4f, // 46.7
            interceptRev : 39.7f,
            inputPowerFactor : 0.72
        },
        {
            min : 50000,
            max : 54000,
            name : (char *)"6M",
            lpf : BAND_6M,
            interceptFwd : 44.7f, // 47.8f
            interceptRev : 40.0f,
            inputPowerFactor : 0.51
        },
        {
            min : 144000,
            max : 148000,
            name : (char *)"VHF",
            lpf : BAND_2M,
            interceptFwd : 42.8f,
            interceptRev : 39.5f,
            inputPowerFactor : 0.20
        },
        {
            min : 430000,
            max : 440000,
            name : (char *)"UHF",
            lpf : BAND_70CM,
            interceptFwd : 39.5f,
            interceptRev : 39.5f,
            inputPowerFactor : 0.073
        },
    };

    Band _lastBand = HAM_BANDS[0];
    float _lastTemperature;
    float _temperatureAccumulator;
    uint8_t _lastFanSpeed = 0;
    timer_t _timer1;
    volatile timer_t _timerRfInput;
    uint8_t _counter;
    time_t _nextTouch;

    volatile bool _started = false;
    volatile bool _connected = false;
    volatile uint32_t _lastFreq = 0;
    volatile bool _txStateBT = false;
    volatile bool _txStateRF = false;
    volatile bool _knownBand = false;
    volatile bool _protectionEnabled = false;
    volatile bool _bypassEnabled = false;
    volatile bool _psuAlarm = false;
    volatile bool _overTemperature = false;
};