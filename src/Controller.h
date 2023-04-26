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
    void updateScreenTimeout();

    float _outputPowerAccumulator = 0;
    float _inputPowerAccumulator = 0;
    float _powerGainDB = 0;
    Preferences _preferences;

    Band bandLookup(uint32_t freq);

    ICOM _rig;
    HardwareLayer _hal;
    CANPSU _psu;

    uint8_t _temperatureMin = 35;
    uint8_t _temperatureMax = 50;
    // ATU1000 _atu;
    UI _ui;

    Band HAM_BANDS[BANDS_SIZE] = {
        {
            min : 0,
            max : 0,
            name : (char *)"-",
            lpf : BAND_OTHER,
            outputPowerFactorFwd : 1,
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
        {
            min : 1800,
            max : 2000,
            name : (char *)"160",
            lpf : BAND_160M,
            outputPowerFactorFwd : 1, 
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
        {
            min : 3500,
            max : 4000,
            name : (char *)"80M",
            lpf : BAND_80M,
            outputPowerFactorFwd : 1, 
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
        {
            min : 6900,
            max : 6999,
            name : (char *)";-)",
            lpf : BAND_60_40M,
            outputPowerFactorFwd : 1, 
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
        {
            min : 7000,
            max : 7300,
            name : (char *)"40M",
            lpf : BAND_60_40M,
            outputPowerFactorFwd : 1, 
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
        {
            min : 10000,
            max : 10500,
            name : (char *)"30M",
            lpf : BAND_30_20M,
            outputPowerFactorFwd : 1,
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
        {
            min : 14000,
            max : 14350,
            name : (char *)"20M",
            lpf : BAND_30_20M,
            outputPowerFactorFwd : 1,
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
        {
            min : 18000,
            max : 18500,
            name : (char *)"17M",
            lpf : BAND_17_15M,
            outputPowerFactorFwd : 1,
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
        {
            min : 21000,
            max : 21500,
            name : (char *)"15M",
            lpf : BAND_17_15M,
            outputPowerFactorFwd : 12.2f, 
            outputPowerFactorRev : 12.2f,
            inputPowerFactor: 1.40f
        },
        {
            min : 24500,
            max : 25000,
            name : (char *)"12M",
            lpf : BAND_12_10M,
            outputPowerFactorFwd : 12.3f, // 45.5
            outputPowerFactorRev : 12.3f,
            inputPowerFactor: 1.43f
        },
        {
            min : 26965,
            max : 27855,
            name : (char *)"11M",
            lpf : BAND_12_10M,
            outputPowerFactorFwd : 12.3f,
            outputPowerFactorRev : 24.0f,
            inputPowerFactor: 1.50f
        },
        {
            min : 28000,
            max : 29700,
            name : (char *)"10M",
            lpf : BAND_12_10M,
            outputPowerFactorFwd : 12.3f,
            outputPowerFactorRev : 24.0f,
            inputPowerFactor: 1.50f
        },
        {
            min : 50000,
            max : 54000,
            name : (char *)"6M",
            lpf : BAND_6M,
            outputPowerFactorFwd : 13.3f, 
            outputPowerFactorRev : 13.3f,
            inputPowerFactor: 2.86f
        },
        {
            min : 144000,
            max : 148000,
            name : (char *)"VHF",
            lpf : BAND_2M,
            outputPowerFactorFwd : 1,
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
        {
            min : 430000,
            max : 440000,
            name : (char *)"UHF",
            lpf : BAND_70CM,
            outputPowerFactorFwd : 1,
            outputPowerFactorRev : 1,
            inputPowerFactor: 1
        },
    };

    Band _lastBand = HAM_BANDS[0];
    float _lastTemperature;
    float _temperatureAccumulator;
    uint8_t _lastFanSpeed = 0;
    timer_t _timer1;
    volatile timer_t _timerTimeout = 0;
    volatile timer_t _timerRfInput;
    uint8_t _counter;
    time_t _nextTouch;
    bool _voltageSet;

    volatile bool _started = false;
    volatile bool _connected = false;
    volatile uint32_t _lastFreq = 0;
    volatile bool _prevScreenOff = false;
    volatile bool _txStateBT = false;
    volatile bool _txStateRF = false;
    volatile bool _knownBand = false;
    volatile bool _protectionEnabled = false;
    volatile bool _bypassEnabled = false;
    volatile bool _psuAlarm = false;
    volatile bool _overTemperature = false;


};