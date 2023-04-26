#include "Controller.h"

Controller::Controller()
{
}

Band Controller::bandLookup(uint32_t freq)
{
    for (uint8_t i = 1; i < BANDS_SIZE; i++)
    {
        if (freq >= HAM_BANDS[i].min && freq <= HAM_BANDS[i].max)
        {
            return HAM_BANDS[i];
        }
    }
    return HAM_BANDS[0];
}

void Controller::onOutputPower(float forwardWatts, float reverseWatts)
{
    if (!_started)
        return;
    if (forwardWatts > 0.0f || _outputPowerAccumulator > 0.0f)
    {
        float alpha = forwardWatts > _outputPowerAccumulator ? 1.0f : 0.2f; // 1 = fast 0.2 = slow
        _outputPowerAccumulator += alpha * (forwardWatts - _outputPowerAccumulator);

        if (_outputPowerAccumulator < 1.0f)
            _outputPowerAccumulator = 0.0f;

        _ui.updateOutputPower(_outputPowerAccumulator, reverseWatts);

        float gain = 0;
        if (_inputPowerAccumulator > 0 && _outputPowerAccumulator > 0)
        {
            gain = (10.0 * log10(_outputPowerAccumulator / _inputPowerAccumulator));
            float ratio = _outputPowerAccumulator / _inputPowerAccumulator;
            // DBG("Controller::onOutputPower: Out Avg %.4fW, In Avg: %.4fW, Gain: %.1fdB, Ratio: %.4f\n", _outputPowerAccumulator, _inputPowerAccumulator, gain, ratio);
        }

        if (gain != _powerGainDB)
        {
            _powerGainDB = gain;
            _ui.updateGain(_powerGainDB);
        }
    }
}

void Controller::onInputPower(float forwardWatts)
{
    if (!_started)
        return;

    if (forwardWatts > 0.1f)
    {
        updateScreenTimeout();
        _timerRfInput = millis() + 100;

        if (!_txStateRF)
        {
            _txStateRF = true;
            DBG("RF Input: TX ON (ms %ld) [Core %d]\n", millis(), xPortGetCoreID());
        }
    }

    if (forwardWatts > 0.1f || _inputPowerAccumulator > 0.0f)
    {
        // DBG("Controller::onInputPower: %.1fW [Core %d]\n", forwardWatts, xPortGetCoreID());
        float alpha = forwardWatts > _inputPowerAccumulator ? 0.5f : 0.2f; // 1 = fast 0.2 = slow
        _inputPowerAccumulator += alpha * (forwardWatts - _inputPowerAccumulator);

        _ui.updateInputPower(_inputPowerAccumulator);
    }
}

void Controller::onButtonPressed(int button, bool longPress)
{
    DBG("onButtonPressed: Button: %d, Long: %s\n", button, longPress ? "true" : "false");
    switch (button)
    {
    case BUTTON_A:
        if (longPress)
        {
            _ui.previousScreen();
        }
        else
        {
            _ui.nextScreen();
        }
        break;
    }
}

void Controller::updateScreenTimeout()
{
    if (_connected)
    {
        _timerTimeout = millis() + SCREEN_TIMEOUT_CONNECTED;
    }
    else
    {
        _timerTimeout = millis() + SCREEN_TIMEOUT_DISCONNECTED;
    }
}

void Controller::onFrequencyChanged(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState)
{
    updateScreenTimeout();
    // DBG("Controller::onFrequencyChanged [Core %d]\n", xPortGetCoreID());
    uint32_t mainfreq = frequency / 1000;
    if (mainfreq != _lastFreq)
    {
        Band band = bandLookup(mainfreq);
        if (band.name != _lastBand.name)
        {
            _hal.onBandChanged(mainfreq, band);
        }
        _lastBand = band;
        _lastFreq = mainfreq;
    }

    _knownBand = (modulation <= 5 || modulation == 23) &&
                 strcmp(_lastBand.name, "-") != 0 &&
                 strcmp(_lastBand.name, "UHF") != 0;

    if (txState != _txStateBT)
    {
        _txStateBT = txState;

        if (transmitEnabled())
            _hal.onTransmitChanged(txState);

        DBG("BT PTT Changed: %s (%lu) [Core %d]\n", txState ? "true" : "false", millis(), xPortGetCoreID());
    }
    _ui.frequencyChanged(frequency, modulation, filter, txState, _lastBand.name, transmitEnabled());
}

void Controller::onClientConnected(uint8_t macAddress[6])
{
    DBG("Controller::onClientConnected [Core %d]\n", xPortGetCoreID());
    _connected = true;
    updateScreenTimeout();
}

void Controller::onPowerSupplyStatus(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage)
{
    if (!_voltageSet && mode == PowerSupplyMode::NORMAL)
    {
        // _psu.setStartupVoltage(50);
        // _psu.setOperationalVoltageAndCurrent(50, 18);
        // _voltageSet = true;
    }
    _psuAlarm = mode != PowerSupplyMode::NORMAL;
    _ui.updatePowerSupply(mode, intTemp, outTemp, current, outVoltage, inputVoltage);
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
    // DBG("Controller::onMeterUpdate: Type: %d, Raw: %d, Value: %d \n", type, rawValue, value);
}

void Controller::onDiagnosticsUpdated(Diag diag)
{
    _ui.updateDiagnostics(diag);
}

void Controller::onTemperatureUpdated(float temperature)
{
    if (_lastTemperature != temperature && _started)
    {
        _ui.updateTemperature(temperature);

        float alpha = temperature > _temperatureAccumulator ? 1 : 0.1f; // 1 = fast 0.1 = slow
        _temperatureAccumulator += alpha * (temperature - _temperatureAccumulator);

        _overTemperature = _temperatureAccumulator > PROTECTION_TEMPERATURE;

        uint8_t t = minimum(_temperatureAccumulator, _temperatureMax);
        t = maximum(t, _temperatureMin);

        uint8_t fanSpeed = map(t, _temperatureMin, _temperatureMax, 0, 100);

        _lastTemperature = temperature;
        if (fanSpeed != _lastFanSpeed)
        {
            _hal.setFanSpeed(fanSpeed);
            _ui.updateFanSpeed(fanSpeed);
            _lastFanSpeed = fanSpeed;
            DBG("Controller: Fan speed set to %d%% (Temp: %.2foC, Avg: %.2foC) [Core %d]\n", fanSpeed, temperature, _temperatureAccumulator, xPortGetCoreID());
        }
    }
}

void Controller::onTouch(TouchPoint tp)
{
    updateScreenTimeout();

    if (_prevScreenOff)
        return;

    if (millis() > _nextTouch)
    {
        TouchCmd cmd = _ui.touch(tp);
        switch (cmd)
        {
        case TouchCmd::TOUCH_NONE:
            break;
        case TouchCmd::MAIN_PSU:
            _ui.loadScreen(Screens::PSU);
            break;
        case TouchCmd::MAIN_BYPASS:
            _bypassEnabled = !_bypassEnabled;
            setBypassState();
            break;
        case TouchCmd::MAIN_STANDBY:

            break;
        case TouchCmd::PSU_BACK:
            _ui.loadScreen(Screens::MAIN);
            break;
        }
        _nextTouch = millis() + 500;
    }
}

bool Controller::transmitEnabled()
{
    return _knownBand &&
           !_bypassEnabled;

    // return _knownBand &&
    //        !_bypassEnabled &&
    //        !_protectionEnabled &&
    //        !_psuAlarm &&
    //        !_overTemperature;
}

void Controller::setBypassState()
{
    _ui.updateBypass(_bypassEnabled);
    _hal.onPowerSupplyChanged(_started && !_bypassEnabled);
    if (_preferences.getBool("bypass") != _bypassEnabled)
    {
        _preferences.putBool("bypass", _bypassEnabled);
        DBG("Preferences: Bypass set to %s\n", _bypassEnabled ? "true" : "false");
    }
}

void Controller::start()
{
    DBG("Controller::start() [Core %d]\n", xPortGetCoreID());
    if (_rig.initializeRig())
    {
        DBG("Controller::start() Transceiver detected successfully\n");
        _ui.loadScreen(Screens::MAIN);
        //_ui.loadScreen(Screens::PSU);
        _started = true;
        setBypassState();
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
    _lastFanSpeed = 0;
    _hal.setFanSpeed(0);
    _hal.onPowerSupplyChanged(false);
    updateScreenTimeout();
}

void Controller::loop()
{
    _hal.loop();
    _psu.loop();

    // start & end functions must run o loop (core 1) to avoid blocking core 0 (bluetooth)
    if (_connected && !_started)
    {
        start();
    }
    if (!_connected && _started)
    {
        end();
    }

    _ui.loop(_started);

    if (_txStateRF && millis() > _timerRfInput)
    {
        _txStateRF = false;
        DBG("RF Input: TX OFF (ms %ld) [Core %d]\n", millis(), xPortGetCoreID());
    }

    bool screenOff = millis() > _timerTimeout;
    if (screenOff != _prevScreenOff)
    {
        _prevScreenOff = screenOff;
        _hal.setBacklightLevel(screenOff ? 0 : DEFAULT_BACKLIGHT_LEVEL);
    }
}

void Controller::begin()
{
    DBG("Controller::begin [Core %d]\n", xPortGetCoreID());
    updateScreenTimeout();
    _preferences.begin("settings", false);
    _ui.begin();
    /*************** RIG BEGIN */
    auto cbConnected = std::bind(&Controller::onClientConnected,
                                 this,
                                 std::placeholders::_1);
    auto cbFrequency = std::bind(&Controller::onFrequencyChanged,
                                 this,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3,
                                 std::placeholders::_4);
    auto cbDisconnected = std::bind(&Controller::onClientDisconnected,
                                    this);
    auto cbMeter = std::bind(&Controller::onMeterUpdated,
                             this,
                             std::placeholders::_1,
                             std::placeholders::_2);

    _rig.onConnectedCallback(cbConnected);
    _rig.onDisconnectedCallback(cbDisconnected);
    _rig.onFrequencyCallback(cbFrequency);
    _rig.onMeterCallback(cbMeter);

    _rig.begin(BT_NAME);

    /*************** RIG END */

    /*************** HAL BEGIN */
    auto cbOutputPwr = std::bind(&Controller::onOutputPower,
                                 this,
                                 std::placeholders::_1,
                                 std::placeholders::_2);
    auto cbInputPwr = std::bind(&Controller::onInputPower,
                                this,
                                std::placeholders::_1);
    auto cbButtons = std::bind(&Controller::onButtonPressed,
                               this,
                               std::placeholders::_1,
                               std::placeholders::_2);
    auto cbDiag = std::bind(&Controller::onDiagnosticsUpdated,
                            this,
                            std::placeholders::_1);

    auto cbTemp = std::bind(&Controller::onTemperatureUpdated,
                            this,
                            std::placeholders::_1);
    auto cbTouch = std::bind(&Controller::onTouch,
                             this,
                             std::placeholders::_1);
    _hal.onOutputRfPowerCallback(cbOutputPwr);
    _hal.onInputRfPowerCallback(cbInputPwr);
    _hal.onButtonPressedCallback(cbButtons);
    _hal.onDiagnosticsCallback(cbDiag);
    _hal.onTemperatureCallback(cbTemp);
    _hal.onTouchCallback(cbTouch);

    Diag diag = _hal.begin();
    /*************** HAL END */

    /*************** PSU BEGIN */
    auto cbPsuStatus = std::bind(&Controller::onPowerSupplyStatus,
                                 this,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3,
                                 std::placeholders::_4,
                                 std::placeholders::_5,
                                 std::placeholders::_6);

    _psu.onStatusCallback(cbPsuStatus);
    _psu.begin();
    /*************** PSU END */

    _bypassEnabled = _preferences.getBool("bypass");

    if (diag.mainExpander && diag.rfAdcFwd && diag.rfAdcRev && diag.temperature)
    {
        _ui.loadScreen(Screens::STANDBY);
    }
    else
    {
        _ui.loadScreen(Screens::DIAG);
    }
}

void Controller::end()
{
    DBG("Controller::end() [Core %d]\n", xPortGetCoreID());
    if (_started)
    {
        _ui.loadScreen(Screens::STANDBY);
    }
    _started = false;
}