#include "CAN/EltekFP2.h"

EltekFP2::~EltekFP2()
{
}

void EltekFP2::begin(CAN *can)
{
    _can = can;
    _started = true;
}

void EltekFP2::end()
{
    _started = false;
    _serialNumberReceived = false;
}

void EltekFP2::logIn()
{
    //DBG("EltekFP2: Logging in\n");
    _can->send(0x05004804, 8, _serialNumber);
}

void EltekFP2::storeSerialNumber(uint32_t rxID, uint8_t len, uint8_t rxBuf[])
{
    uint8_t offset = 1; // 0x0500XXXX Serial numbers starts on the second byte
    if (rxID == 0x05014400)
    {
        offset = 0;
    }
    DBG("EltekFP2: Found power supply: ");
    memset(_serialNumber, 0, sizeof(_serialNumber));
    memcpy(_serialNumber, rxBuf + offset, 6);
    for (uint8_t i = 0; i < 6; i++)
    {
        DBG("%02X ", _serialNumber[i]);
    }
    DBG("\n");
    _serialNumberReceived = true;
}

void EltekFP2::processStatusMessage(uint32_t rxID, uint8_t len, uint8_t rxBuf[])
{
    if (!_serialNumberReceived)
        return;

    PowerSupplyMode currentMode = PowerSupplyMode::NORMAL;

    int intakeTemperature = rxBuf[0];
    float current = 0.1f * (rxBuf[1] | (rxBuf[2] << 8));
    float outputVoltage = 0.01f * (rxBuf[3] | (rxBuf[4] << 8));
    int inputVoltage = rxBuf[5] | (rxBuf[6] << 8);
    int outputTemperature = rxBuf[7];

#ifdef DEBUG
    // if (millis() > _lastDebug)
    // {
    //     _lastDebug = millis() + 5000;
    //     DBG("EltekFP2: Status message\n");
    //     DBG("   Temperature ....: In: %dºC, Out: %dºC\n", intakeTemperature, outputTemperature);
    //     DBG("   Current ........: %.2fA\n", current);
    //     DBG("   Output Voltage .: %.2fV\n", outputVoltage);
    //     DBG("   Input Voltage ..: %dV\n\n", inputVoltage);
    // }
#endif
    outputVoltage = ((int)(outputVoltage * 10)) / 10.0f; // truncate to 1 decimal digit

    if (rxID == 0x05014010)
    {
        currentMode = PowerSupplyMode::RAMPING;
        //DBG("EltekFP2: Currently in walk in (voltage ramping up)\n");
    }

    bool hasWarning = rxID == 0x05014008;
    bool hasAlarm = rxID == 0x0501400C;

    if (hasWarning || hasAlarm)
    {
        //DBG("EltekFP2: %s notification received, asking for details\n", hasWarning ? "Warning" : "Alarm");
        uint8_t buff[3] = {(uint8_t)0x08, (uint8_t)(hasWarning ? 0x04 : 0x08), 0x00};
        _can->send(0x0501BFFC, 3, buff);
        if (hasWarning)
        {
            currentMode = PowerSupplyMode::WARNING;
        }
        else
        {
            currentMode = PowerSupplyMode::ALARM;
        }
    }

    if (_mode != currentMode || _outputVoltage != outputVoltage || _current != current)
    {
        // DBG("EltekFP2: Someting changed, firing event\n");
        if (_statusCb && outputVoltage < 100) // sanity check
        {
            _statusCb(currentMode, intakeTemperature, outputTemperature, current, outputVoltage, inputVoltage);
        }
    }

    _mode = currentMode;
    _intakeTemperature = intakeTemperature;
    _current = current;
    _outputVoltage = outputVoltage;
    _inputVoltage = inputVoltage;
    _outputTemperature = outputTemperature;
}

void EltekFP2::processWarningOrAlarmMessage(uint32_t rxID, uint8_t len, uint8_t rxBuf[])
{
    if (!_serialNumberReceived)
        return;

    bool isWarning = rxBuf[1] == 0x04;
    //DBG("EltekFP2: %s: ", isWarning ? "Warning" : "Alarm");

    uint8_t alarms0 = rxBuf[3];
    uint8_t alarms1 = rxBuf[4];

    for (int i = 0; i < 8; ++i)
    {
        if (alarms0 & (1 << i))
        {
            //DBG(" %s", _alarms0Strings[i]);
        }

        if (alarms1 & (1 << i))
        {
            //DBG(" %s", _alarms1Strings[i]);
        }
    }
    //DBG("\n");
}

void EltekFP2::onStatusCallback(PowerSupplyStatusCb cb)
{
    _statusCb = cb;
}

void EltekFP2::onAlarmCallback(PowerSupplyAlarmCb cb)
{
    _alarmCb = cb;
}

void EltekFP2::setStartupVoltage(float volts)
{
    if (!_serialNumberReceived)
        return;

    uint16_t voltage = volts * 100;
    uint8_t voltsB1 = (uint8_t)((voltage >> 8) & 0xFF);
    uint8_t voltsB2 = (uint8_t)(voltage & 0xFF);
    uint8_t voltageSetTxBuf[5] = {0x29, 0x15, 0x00, voltsB2, voltsB1};
    _can->send(0x05019C00, 5, voltageSetTxBuf);
    DBG("EltekFP2: Voltage set to %.2f\n", volts);
}

void EltekFP2::setOperationalVoltageAndCurrent(float volts, float amps)
{
    if (!_serialNumberReceived)
        return;

    uint16_t voltage = volts * 100;
    uint8_t voltsB1 = (uint8_t)((voltage >> 8) & 0xFF);
    uint8_t voltsB2 = (uint8_t)(voltage & 0xFF);

    uint16_t voltageOvp = (volts + 2) * 100;
    uint8_t voltsOvpB1 = (uint8_t)((voltageOvp >> 8) & 0xFF);
    uint8_t voltsOvpB2 = (uint8_t)(voltageOvp & 0xFF);

    uint16_t amperes = amps * 10;
    uint8_t ampsB1 = (uint8_t)((amperes >> 8) & 0xFF);
    uint8_t ampsB2 = (uint8_t)(amperes & 0xFF);

    // example: set rectifier to 16.0 amps (00 A0) 57.60 (16 80) and OVP to 59.5V (17 3E)
    unsigned char voltageSetTxBuf[8] = {ampsB2, ampsB1, voltsB2, voltsB1, voltsB2, voltsB1, voltsOvpB2, voltsOvpB1};

    _can->send(0x05FF4004, 8, voltageSetTxBuf);

    DBG("EltekFP2: Operational voltage set to %.2fVolts @ %.1fAmps\n", volts, amps);
}

float EltekFP2::maxCurrent()
{
    return _inputVoltage > 150 ? ELTEK_MAX_CURRENT_220 : ELTEK_MAX_CURRENT_110;
}

void EltekFP2::processMessage(uint32_t id, uint8_t len, uint8_t *data)
{
    _lastUpdate = millis();
    // DBG("EltekFP2: Processing message 0x%03X [%d]\n", id, len);

    if ((id & 0xFFFF0000) == 0x05000000 || id == 0x05014400)
    {
        if (!_serialNumberReceived)
        {
            storeSerialNumber(id, len, data);
        }
        logIn();
    }
    else if ((id & 0xFFFFFF00) == 0x05014000)
    {
        processStatusMessage(id, len, data);
    }
    else if (id == 0x0501BFFC)
    {
        processWarningOrAlarmMessage(id, len, data);
    }
}

void EltekFP2::loop()
{
    if (!_started)
        return;

    if (_mode > PowerSupplyMode::POWEROFF && (millis() - _lastUpdate) > 1000)
    {
        _mode = PowerSupplyMode::POWEROFF;
        if (_statusCb)
        {
            _statusCb(_mode, 0, 0, 0, 0, 0);
        }
    }
}
