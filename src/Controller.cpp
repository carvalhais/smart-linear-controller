#include "Controller.h"

Controller::Controller()
{
}

void Controller::onAmplifierCallback(AmplifierCb callback)
{
    _amplifierCallback = callback;
}
void Controller::onLowPassFilterCallback(LowPassFilterCb callback)
{
    _lowPassFilterCallback = callback;
}

void Controller::onTransmitCallback(TransmitCb callback)
{
    _transmitCallback = callback;
}

void Controller::onPowerSupplyCallback(PowerSupplyCb callback)
{
    _psuCallback = callback;
}

Band Controller::bandLookup(uint32_t freq)
{
    for (uint8_t i = 1; i < BANDS_SIZE; i++)
    {
        if (freq >= _bands[i].min && freq <= _bands[i].max)
        {
            return _bands[i];
        }
    }
    return _bands[0];
}

void Controller::onInputSwr(float forwardMv, float reverseMv)
{
    if (!_started)
        return;
    bool txState = forwardMv > 0;
    if (txState != _lastRFTxState)
    {
        _lastRFTxState = txState;
        // Serial.printf("RF PTT Changed: %s (%lu)\n", txState ? "true" : "false", millis());
    }
    _meters.updateInputSwr(forwardMv, reverseMv);
}

void Controller::onOutputSwr(float forwardMv, float reverseMv)
{
    if (!_started)
        return;
    _meters.updateOutputSwr(forwardMv, reverseMv);
}

void Controller::onButtonPressed(int button, bool longPress)
{
    DBG("onButtonPressed: Button: %d, Long: %s\n", button, longPress ? "true" : "false");
}

void Controller::onFrequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState)
{
    // DBG("Controller::onFrequencyChanged [Core %d]\n", xPortGetCoreID());

    uint32_t mainfreq = frequency / 1000;

    if (mainfreq != _lastFreq)
    {
        _lastBand = bandLookup(mainfreq);
        _lastFreq = mainfreq;

        Amplifier amp = mainfreq >= FREQ_VHF_AMP ? AMP_VHF : AMP_HF;

        if (amp != _lastAmp)
        {
            _lastAmp = amp;
            if (_amplifierCallback)
                _amplifierCallback(_lastAmp);
        }

        if (_lastAmp == AMP_HF && _lastBand.lpf != _lastLpf)
        {
            _lastLpf = _lastBand.lpf;
            if (_lowPassFilterCallback)
                _lowPassFilterCallback(_lastLpf);
        }
    }

    _transmitEnabled = modulation <= 5 &&
                       strcmp(_lastBand.name, "-") != 0 &&
                       strcmp(_lastBand.name, "UHF") != 0;

    if (txState != _lastBTTxState)
    {
        _lastBTTxState = txState;

        if (_transmitCallback && _transmitEnabled)
            _transmitCallback(txState);

        // DBG("BT PTT Changed: %s (%lu)\n", txState ? "true" : "false", millis());
    }

    _freq.frequencyChanged(frequency, modulation, filter, txState, _lastBand.name, _transmitEnabled);
}

void Controller::onClientConnected(uint8_t macAddress[6])
{
    DBG("Controller::onClientConnected [Core %d]\n", xPortGetCoreID());
    _connected = true;
    if (_psuCallback)
        _psuCallback(true);
}

void Controller::onMeterUpdated(uint8_t type, uint8_t rawValue)
{
    switch (type)
    {
    case CMD_SUB_S_METER:
        uint8_t value = rawValue > 120 ? 90 + map(rawValue, 121, 241, 0, 60) : map(rawValue, 0, 120, 0, 90);
        value = value / 150.0 * 100;
        break;
    }

    //_analog.update(value);
    // DBG("Controller::onMeterUpdate: Type: %d, Raw: %d, Value: %d \n", type, rawValue, value);
}

void Controller::start()
{
    DBG("Controller::start() [Core %d]\n", xPortGetCoreID());

    if (_rig->initializeRig())
    {
        DBG("Controller::start() Transceiver detected successfully\n");

        // fill the body region to clear any previous content
        _tft.fillRect(1, HEADER_HEIGHT + 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - HEADER_HEIGHT - 2, TFT_BLACK);

        // initialize the Frequency Widget
        _freq.begin(1, HEADER_HEIGHT + 1, SCREEN_WIDTH - 2, 40, &_tft, EurostileNextProWide13, EurostileNextProNr18, EurostileNextProSemiBold32);

        // initialize the bargraphs (input and output meters)
        _meters.begin(1, 61, SCREEN_WIDTH - 2, 119, &_tft, Tahoma9Sharp, EurostileNextProNr18);

        // "Digital" meters at the bottom
        _bottom.begin(1, 180, SCREEN_WIDTH - 2, 40, &_tft, Tahoma9Sharp, EurostileNextProNr18);
        _bottom.updateVolts(53);
        _bottom.updateAmperes(0.1);
        _bottom.updateTemperature(56);
        _bottom.updateAntenna((char *)"ANT A");

        //_analog.begin(&_tft, 2, 100, 316, 118);

        _started = true;
    }
    else
    {
        DBG("Controller::start() Unable to initialize transceiver\n");
    }
}

void Controller::onClientDisconnected()
{
    DBG("Controller::onClientDisconnected [Core %d]\n", xPortGetCoreID());
    _connected = false;
    if (_psuCallback)
        _psuCallback(false);
}

void Controller::loop()
{
    // start & end functions must run o loop (core 1) to avoid blocking core 0 (bluetooth)
    if (_connected && !_started)
    {
        start();
    }
    if (!_connected && _started)
    {
        end();
    }
    if (_started)
    {
        _freq.loop();
        _meters.loop();
        //_analog.loop();
    }
}

void Controller::begin(ICOM *rig)
{
    DBG("Controller::begin [Core %d]\n", xPortGetCoreID());
    _rig = rig;
    _tft.init();
#ifdef M5STACK
    _tft.invertDisplay(true);
#endif
    _tft.setRotation(1); // 3
    _tft.fillScreen(TFT_BLACK);

    _ui.begin(&_tft, EurostileNextProWide13);
    _ui.drawHeader();
    _ui.clearScreen();
}

void Controller::end()
{
    DBG("Controller::end() [Core %d]\n", xPortGetCoreID());
    if (_started)
    {
        _ui.clearScreen();
        _freq.end();
        _meters.end();
        _bottom.end();
    }
    _started = false;
}