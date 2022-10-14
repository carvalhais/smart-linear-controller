#include "CAN/HuaweiR48X.h"

HuaweiR48X::~HuaweiR48X()
{
}

void HuaweiR48X::begin(CAN *can)
{
    _can = can;
    _started = true;
    _mode = STARTED;
}

void HuaweiR48X::end()
{
    _started = false;
}

void HuaweiR48X::onStatusCallback(PowerSupplyStatusCb cb)
{
    _statusCb = cb;
}

void HuaweiR48X::onAlarmCallback(PowerSupplyAlarmCb cb)
{
    _alarmCb = cb;
}

void HuaweiR48X::setStartupVoltage(float volts)
{
    uint16_t value = volts * 1024;
    // printf("Voltage = 0x%04X\n",value);
    uint8_t command = 0x01; // Off-line mode
    uint8_t data[8];
    data[0] = 0x01;
    data[1] = command;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = (value & 0xFF00) >> 8;
    data[7] = value & 0xFF;
    _can->send(0x108180FE, 8, data);
    DBG("HuaweiR48X: Voltage set to %.2f\n", volts);
}

void HuaweiR48X::setOperationalVoltageAndCurrent(float volts, float amps)
{
    setOperationalVoltage(volts);
    setOperationalCurrent(amps);
}

void HuaweiR48X::setOperationalVoltage(float volts)
{
    uint16_t value = volts * 1024;
    // printf("Voltage = 0x%04X\n",value);
    uint8_t command = 0x00; // On-line mode
    uint8_t data[8];
    data[0] = 0x01;
    data[1] = command;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = (value & 0xFF00) >> 8;
    data[7] = value & 0xFF;
    _can->send(0x108180FE, 8, data);
    DBG("HuaweiR48X: Operational voltage set to %.2f Volts\n", volts);
}

void HuaweiR48X::setOperationalCurrent(float amps)
{
    uint16_t value = amps * MAX_CURRENT_MULTIPLIER;
    // printf("Current = 0x%04X\n",value);
    uint8_t command = 0x03; // On-line mode
    uint8_t data[8];
    data[0] = 0x01;
    data[1] = command;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = (value & 0xFF00) >> 8;
    data[7] = value & 0xFF;
    _can->send(0x108180FE, 8, data);
    DBG("HuaweiR48X: Operational current set to %.2f Amps\n", amps);
}

float HuaweiR48X::maxCurrent()
{
    return _inputVoltage > 150 ? HUAWEI_MAX_CURRENT_220 : HUAWEI_MAX_CURRENT_110;
}

void HuaweiR48X::loop()
{
    if (!_started)
        return;

    if (millis() > _nextPool)
    {
        requestData();
        _nextPool = millis() + _poolingInterval;
    }

    if (_mode > PowerSupplyMode::POWEROFF && (millis() - _lastUpdate) > 1000)
    {
        _mode = PowerSupplyMode::POWEROFF;
        if (_statusCb)
        {
            _statusCb(_mode, 0, 0, 0, 0, 0);
        }
    }

#ifdef DEBUG
    // if (_mode > PowerSupplyMode::POWEROFF && millis() > _lastDebug)
    // {
    //     _lastDebug = millis() + 5000;
    //     DBG("HuaweiR48X: Status message\n");
    //     DBG("   Temperature ....: In: %dºC, Out: %dºC\n", _intakeTemperature, _outputTemperature);
    //     DBG("   Current ........: %.2fA (Max %.2f)\n", _current, _maxCurrent);
    //     DBG("   Output Voltage .: %.2fV\n", _outputVoltage);
    //     DBG("   Input Voltage ..: %dV\n\n", _inputVoltage);
    // }
#endif
}

void HuaweiR48X::processMessage(uint32_t id, uint8_t len, uint8_t *data)
{
    _lastUpdate = millis();

    // DBG("HuaweiR48X: Processing message 0x%03X [%d]\n", id, len);
    switch (id)
    {
    case 0x1081407F:
        // CANPSU::printMessage(id, len, data);
        handleDataFrame(data);
        break;
    case 0x1081407E:
        /* Acknowledgment */
        break;
    case 0x1081D27F:
        handleDescription(data);
        break;
    case 0x1081807E:
        /* Acknowledgement */
        handleAck(data);
        break;
    case 0x1001117E:
        // r4850_Ahr((uint8_t *)&frame.data, &rp);
        /* Normally 00 01 00 0s 00 00 xx xx */
        /* xx = Whr meter, send every 377mS */
        /* s = 1 when output disabled */
        break;
    case 0x100011FE:
        /* Normally 00 02 00 00 00 00 00 00 */
        break;
    case 0x108111FE:
        /* Normally 00 03 00 00 00 0s 00 00 */
        /* s = 1 when output disabled */
        // if (frame.data[5] == 1)
        //	printf("Output Enabled\n");
        // else 	printf("Output Disabled\n");
        break;
    case 0x108081FE:
        /* Normally 01 13 00 01 00 00 00 00 */
        break;
    default:
        printf("Unknown frame 0x%03X [%d]\n", id, len);
    }
}

void HuaweiR48X::requestData()
{
    if ((millis() - _lastUpdate) < 1000)
    {
        // DBG("HuaweiR48X: Requesting data\n");
        uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};
        _can->send(0x108040FE, 8, data);
    }
}

void HuaweiR48X::handleDataFrame(uint8_t *frame)
{
    uint32_t value = __builtin_bswap32(*(uint32_t *)&frame[4]);
    switch (frame[1])
    {
    case R48xx_DATA_INPUT_POWER:
        // rp->input_power = value / 1024.0;
        break;

    case R48xx_DATA_INPUT_FREQ:
        // rp->input_frequency = value / 1024.0;
        break;

    case R48xx_DATA_INPUT_CURRENT:
        // rp->input_current = value / 1024.0;
        break;

    case R48xx_DATA_OUTPUT_POWER:
        // rp->output_power = value / 1024.0;
        break;

    case R48xx_DATA_EFFICIENCY:
        // rp->efficiency = value / 1024.0;
        break;

    case R48xx_DATA_OUTPUT_VOLTAGE:
    {
        // rp->output_voltage = value / 1024.0;
        float volts = value / 1024.0;
        if (millis() > _nextVoltSample)
        {
            float diff = _outputVoltage > 0 ? ((volts / _outputVoltage) - 1) * 100 : 100;
            if (diff < -0.5 || volts < 40)
            {
                _mode = WARNING;
            }
            else if (diff > 0.5)
            {
                _mode = RAMPING;
            }
            else
            {
                _mode = NORMAL;
            }
            // DBG("HuaweiR48X: Volts: %.2f, Previous: %.2f, Diff: %.2f%% Mode: %d\n", volts, _outputVoltage, diff, _mode);
            _outputVoltage = volts;
            _nextVoltSample = millis() + 500;
        }
        break;
    }

    case R48xx_DATA_OUTPUT_CURRENT_MAX:
        // rp->max_output_current = value / MAX_CURRENT_MULTIPLIER;
        _maxCurrent = value / MAX_CURRENT_MULTIPLIER;
        break;

    case R48xx_DATA_INPUT_VOLTAGE:
        // rp->input_voltage = value / 1024.0;
        _inputVoltage = value / 1024.0;
        break;

    case R48xx_DATA_OUTPUT_TEMPERATURE:
        // rp->output_temp = value / 1024.0;
        _outputTemperature = value / 1024.0;
        break;

    case R48xx_DATA_INPUT_TEMPERATURE:
        // rp->input_temp = value / 1024.0;
        _intakeTemperature = value / 1024.0;
        break;

    case R48xx_DATA_OUTPUT_CURRENT1:
        // printf("Output Current(1) %.02fA\r\n", value / 1024.0);
        // rp->output_current = value / 1024.0;
        //_current = value / 1024.0;
        break;

    case R48xx_DATA_OUTPUT_CURRENT:
        _current = value / 1024.0;
        if (_statusCb)
            _statusCb(_mode, _intakeTemperature, _outputTemperature, _current, _outputVoltage, _inputVoltage);
        /* This is normally the last parameter received. Print */
        // r4850_print_parameters(rp);
        break;

    default:
        // printf("Unknown parameter 0x%02X, 0x%04X\r\n",frame[1], value);
        break;
    }
}

void HuaweiR48X::handleAck(uint8_t *frame)
{
    bool error = frame[0] & 0x20;
    uint32_t value = __builtin_bswap32(*(uint32_t *)&frame[4]);

    switch (frame[1])
    {
    case 0x00:
        DBG("HuaweiR48X: %s setting on-line voltage to %.02fV\n", error ? "Error" : "Success", value / 1024.0);
        break;
    case 0x01:
        DBG("HuaweiR48X: %s setting non-volatile (off-line) voltage to %.02fV\n", error ? "Error" : "Success", value / 1024.0);
        break;
    case 0x02:
        DBG("HuaweiR48X: %s setting overvoltage protection to %.02fV\n", error ? "Error" : "Success", value / 1024.0);
        break;
    case 0x03:
        DBG("HuaweiR48X: %s setting on-line current to %.02fA\n", error ? "Error" : "Success", (float)value / MAX_CURRENT_MULTIPLIER);
        break;
    case 0x04:
        DBG("HuaweiR48X: %s setting non-volatile (off-line) current to %.02fA\n", error ? "Error" : "Success", (float)value / MAX_CURRENT_MULTIPLIER);
        break;
    default:
        DBG("HuaweiR48X: %s setting unknown parameter (0x%02X)\n", error ? "Error" : "Success", frame[1]);
        break;
    }
}

void HuaweiR48X::handleDescription(uint8_t *frame)
{
    DBG("%c%c%c%c%c%c", frame[2], frame[3], frame[4], frame[5], frame[6], frame[7]);
}
