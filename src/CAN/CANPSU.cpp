#include <CAN/CANPSU.h>

CANPSU::CANPSU()
{
}

void CANPSU::begin()
{
    _started = _can.begin(PIN_CAN_TX, PIN_CAN_RX);
    if (_started && _statusCb)
    {
        _statusCb(PowerSupplyMode::STARTED, 0, 0, 0, 0, 0);
    }
}

void CANPSU::end()
{
    if (_started)
        _can.end();

    if (_detected)
    {
        _handler->end();
    }

    _detected = false;
}

void CANPSU::onStatusCallback(PowerSupplyStatusCb cb)
{
    _statusCb = cb;
}

void CANPSU::onAlarmCallback(PowerSupplyAlarmCb cb)
{
    _alarmCb = cb;
}

void CANPSU::onPowerSupplyStatus(PowerSupplyMode mode, int intTemp, int outTemp, float current, float outVoltage, int inputVoltage)
{
    if (_detected && _mode != mode && mode == PowerSupplyMode::POWEROFF)
    {
        DBG("CANPSU: Power Off\n");
        _detected = false;
        _handler->end();
        _handler = nullptr;
    }

    if (_statusCb)
        _statusCb(mode, intTemp, outTemp, current, outVoltage, inputVoltage);
}

void CANPSU::setStartupVoltage(float volts)
{
    if (_detected)
    {
        _handler->setStartupVoltage(volts);
    }
}

void CANPSU::setOperationalVoltageAndCurrent(float volts, float amps)
{
    if (_detected)
    {
        _handler->setOperationalVoltageAndCurrent(volts, amps);
    }
}

float CANPSU::maxCurrent()
{
    if (_detected)
    {
        return _handler->maxCurrent();
    }
    return 0;
}

void CANPSU::loop()
{
    if (!_started)
        return;

    PowerSupplyMessage message;
    while (_can.receive(&message))
    {
        //printMessage(message.id, message.len, message.data);

        uint32_t id = message.id;
        // Limit ID to lowest 29 bits (extended CAN)
        id &= 0x1FFFFFFF;

        if (!_detected)
        {
            switch (id & 0xFF000000)
            {
            case 0x05000000:
                DBG("Eltek PSU detected: CAN ID: 0x%08X\n", id);
                _handler = new EltekFP2();
                _detected = true;
                break;
            case 0x10000000:
                DBG("Huawei PSU detected: CAN ID: 0x%08X\n", id);
                _handler = new HuaweiR48X();
                _detected = true;
                break;
            default:
                DBG("Unknown CAN Message ID: 0x%08X\n", id);
            }

            printMessage(id, message.len, message.data);

            if (_detected)
            {
                /*************** PSU BEGIN */
                auto cbPsuStatus = std::bind(&CANPSU::onPowerSupplyStatus,
                                             this,
                                             std::placeholders::_1,
                                             std::placeholders::_2,
                                             std::placeholders::_3,
                                             std::placeholders::_4,
                                             std::placeholders::_5,
                                             std::placeholders::_6);

                //_handler->setStartupVoltage(54);
                _handler->onAlarmCallback(_alarmCb);
                //_handler->onStatusCallback(_statusCb);
                _handler->onStatusCallback(cbPsuStatus);
                _handler->begin(&_can);
            }
        }
        if (_detected)
        {
            _handler->processMessage(id, message.len, (uint8_t *)&message.data);
        }
    }
    if (_detected)
    {
        _handler->loop();
    }
}

void CANPSU::printMessage(uint32_t id, uint8_t len, uint8_t *data)
{
#ifdef DEBUG
    char output[256];
    snprintf(output, 256, "ID: 0x%08x Length: %d Data:", id, len);
    Serial.print(output);
    for (int i = 0; i < len; ++i)
    {
        snprintf(output, 256, " 0x%.2X", data[i]);
        Serial.print(output);
    }
    Serial.println();
#endif
}
