#include <TFT_eSPI.h>
#include <SPI.h>

#include "Defines.h"
#include "Types.h"

#include "Fonts/Tahoma9Sharp.h"
#include "Fonts/EurostileNextProWide13.h"
#include "Fonts/EurostileNextProNr18.h"
#include "Fonts/EurostileNextProSemiBold32.h"

#include "ICOM.h"
#include "UI/UI.h"
#include "UI/Frequency.h"
#include "UI/Meters.h"
#include "UI/Bottom.h"

#define BANDS_SIZE 15

typedef std::function<void(Amplifier amp)> AmplifierCb;
typedef std::function<void(LowPassFilter lpf)> LowPassFilterCb;
typedef std::function<void(bool state)> TransmitCb;
typedef std::function<void(bool state)> PowerSupplyCb;

class Controller
{
public:
    Controller();
    void begin(ICOM *rig);
    void end();
    void loop();

    void onFrequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState);
    void onClientConnected(uint8_t macAddress[6]);
    void onClientDisconnected();

    void onAmplifierCallback(AmplifierCb callback);
    void onLowPassFilterCallback(LowPassFilterCb callback);
    void onTransmitCallback(TransmitCb callback);
    void onPowerSupplyCallback(PowerSupplyCb callback);

    void onInputSwr(float forwardMv, float reverseMv);
    void onOutputSwr(float forwardMv, float reverseMv);

private:
    void updateFrequencyWidget();
    void start();

    Band bandLookup(uint32_t freq);

    TFT_eSPI _tft = TFT_eSPI();
    ICOM *_rig;
    UI _ui;
    Frequency _freq;
    Meters _meters;
    Bottom _bottom;

    Band _bands[BANDS_SIZE] = {
        {
            min : 0,
            max : 0,
            name : (char *)"-",
            lpf : BAND_OTHER
        },
        {
            min : 1800,
            max : 2000,
            name : (char *)"160",
            lpf : BAND_160M

        },
        {
            min : 3500,
            max : 4000,
            name : (char *)"80M",
            lpf : BAND_80M
        },
        {
            min : 6900,
            max : 6999,
            name : (char *)";-)",
            lpf : BAND_60_40M
        },
        {
            min : 7000,
            max : 7300,
            name : (char *)"40M",
            lpf : BAND_60_40M
        },
        {
            min : 10000,
            max : 10500,
            name : (char *)"30M",
            lpf : BAND_30_20M
        },
        {
            min : 14000,
            max : 14350,
            name : (char *)"20M",
            lpf : BAND_30_20M
        },
        {
            min : 18000,
            max : 18500,
            name : (char *)"17M",
            lpf : BAND_17_15_12M
        },
        {
            min : 21000,
            max : 21500,
            name : (char *)"15M",
            lpf : BAND_17_15_12M
        },
        {
            min : 24500,
            max : 25000,
            name : (char *)"12M",
            lpf : BAND_17_15_12M
        },
        {
            min : 26965,
            max : 27855,
            name : (char *)"11M",
            lpf : BAND_10_11M
        },
        {
            min : 28000,
            max : 29700,
            name : (char *)"10M",
            lpf : BAND_10_11M
        },
        {
            min : 50000,
            max : 54000,
            name : (char *)"6M",
            lpf : BAND_6M
        },
        {
            min : 144000,
            max : 148000,
            name : (char *)"VHF",
            lpf : BAND_2M
        },
        {
            min : 430000,
            max : 440000,
            name : (char *)"UHF",
            lpf : BAND_70CM
        },
    };

    Band _lastBand = _bands[0];
    Amplifier _lastAmp = AMP_UNKNOWN;
    LowPassFilter _lastLpf = BAND_OTHER;

    AmplifierCb _amplifierCallback;
    LowPassFilterCb _lowPassFilterCallback;
    TransmitCb _transmitCallback;
    PowerSupplyCb _psuCallback;

    volatile bool _started = false;
    volatile bool _connected = false;
    volatile uint32_t _lastFreq = 0;
    volatile bool _lastBTTxState = false;
    volatile bool _lastRFTxState = false;
    volatile bool _transmitEnabled = false;
};