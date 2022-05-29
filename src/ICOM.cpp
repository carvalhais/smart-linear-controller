#include "ICOM.h"

ICOM *ICOM::instance;

ICOM::ICOM()
{
    instance = this;
}

void ICOM::onFrequencyCallback(FrequencyCb callback)
{
    _frequencyCallback = callback;
}

void ICOM::onConnectedCallback(ClientConnectedCb callback)
{
    _clientConnectedCallback = callback;
}

void ICOM::onDisconnectedCallback(ClientDisconnectedCb callback)
{
    _clientDisconnectedCallback = callback;
}

void ICOM::onMeterCallback(MeterCb callback)
{
    _meterCallback = callback;
}

void ICOM::onData(const uint8_t *buffer, size_t size)
{
#ifdef DEBUG
    // dumpBuffer((uint8_t *)buffer, size);
#endif
    size_t offset = 0;
    for (size_t n = 0; n < size; n++)
    {
        if (buffer[n] == STOP_BYTE || buffer[n] == 0xFF)
        {
            size_t size = n - offset + 1;
            if (size <= BT_BUFFER_SIZE)
            {
                uint8_t chunk[size];
                memcpy(chunk, buffer + offset, size);
                offset += size;
                handleNextMessage(chunk, size);
            }
        }
    }
}

void ICOM::begin(String bluetoothName)
{
    bt.begin(bluetoothName);

    // wrapper for "C-styled" callbacks. The onData event is already ported to C++ std::bind paradigm
    bt.register_callback(+[](esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
                         { ICOM::instance->eventCallback(event, param); });

    auto callbackData = std::bind(&ICOM::onData, this, std::placeholders::_1, std::placeholders::_2);
    bt.onData(callbackData);
}

void ICOM::eventCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event)
    {
    case ESP_SPP_SRV_OPEN_EVT:
        if (_clientConnectedCallback)
        {
            _clientConnectedCallback(param->srv_open.rem_bda);
        }
        break;
    case ESP_SPP_CLOSE_EVT:
        _radioAddress = 0x00;
        if (_clientDisconnectedCallback)
        {
            _clientDisconnectedCallback();
        }
        break;
    case ESP_SPP_DATA_IND_EVT: // Data Received: Useless as it dont flush internal buffer
        // readBluetoothBuffer();
        // Serial.printf("BT Data Received: %d\n", param->data_ind.len);
        break;
    case ESP_SPP_WRITE_EVT: // Write complete

        break;
    default:
        DBG("BT Event: %d\n", event);
    }
}

void ICOM::dumpBuffer(uint8_t *buffer, uint8_t size)
{
    Serial.print("<< ");
    for (uint8_t i = 0; i < size; i++)
    {
        Serial.printf("%02X ", buffer[i]);
        if ((buffer[i] == STOP_BYTE || buffer[i] == 0xFF) && i < (size - 1))
            Serial.printf("\n<< ");
    }
    Serial.println();
}

void ICOM::sendCodeRequest(uint8_t requestCode)
{
    uint8_t req[] = {START_BYTE, START_BYTE, _radioAddress, CONTROLLER_ADDRESS, requestCode, STOP_BYTE};
    sendRawRequest(req, sizeof(req));
}

void ICOM::sendRawRequest(uint8_t request[], uint8_t size)
{
    bt.write(request, size);
    bt.flush();
}

void ICOM::handleNextMessage(uint8_t *buffer, uint8_t size)
{
    // <FE FE E0 42 04 00 01 FD  - LSB
    // <FE FE E0 42 03 00 00 58 45 01 FD  -145.580.000
    // FE FE - start bytes
    // 00/E0 - target address (broadcast/controller)
    // 42 - source address
    // 00/03 - data type
    // <data>
    // FD - stop byte

    bool knownCommand = true;
    bool updateFrequency = false;

    if (size < 5)
    {
        DBG("Invalid command size. Expected: %d, Got: %d\n", 5, size);
        return;
    }

    if (buffer[0] == START_BYTE && buffer[1] == START_BYTE)
    {
        if (buffer[3] == _radioAddress || (_radioAddress == 0x00 && buffer[4] == CMD_READ_FREQ))
        {
            if (_radioAddress == 0x00 && buffer[3] != 0x00)
            {
                _radioAddress = buffer[3];
                DBG("Got radio address: %2X\n", _radioAddress);
            }

            // DBG("Radio address: %2X, CMD: %2X\n", buffer[3], buffer[4]);

            if (buffer[2] == CONTROLLER_ADDRESS || buffer[2] == BROADCAST_ADDRESS)
            {
                switch (buffer[4])
                {
                case CMD_TRANS_FREQ:
                case CMD_READ_FREQ:
                    // Serial.println("READ_FREQ");
                    if (size < 10) // incomplete command
                    {
                        DBG("Invalid command size. Command: READ_FREQ, Expected: %d, Got: %d\n", 10, size);
                        return;
                    }
                    _frequency = 0;
                    for (uint8_t i = 0; i < 5; i++)
                    {
                        if (buffer[9 - i] == 0xFD)
                            continue; // spike
                        _frequency += (buffer[9 - i] >> 4) * decMulti[i * 2];
                        _frequency += (buffer[9 - i] & 0x0F) * decMulti[i * 2 + 1];
                    }
                    updateFrequency = true;
                    break;
                case CMD_TRANS_MODE:
                case CMD_READ_MODE:
                    if (size < 7) // incomplete command
                    {
                        DBG("Invalid command size. Command: READ_MODE, Expected: %d, Got: %d\n", 7, size);
                        return;
                    }
                    // Serial.println("READ_MODE");
                    _modulation = buffer[5]; // FE FE E0 42 04 <00 01> FD
                    _filter = buffer[6];     // 01 - Wide, 02 - Medium, 03 - Narrow
                    updateFrequency = true;
                    break;
                case CMD_TRANSMIT_STATE:
                    // Serial.println("TRANSMIT_STATE");
                    if (size < 8) // incomplete command
                    {
                        DBG("Invalid command size. Command: TRANSMIT_STATE, Expected: %d, Got: %d\n", 8, size);
                        return;
                    }
                    _txState = buffer[7] == 0x01;
                    updateFrequency = true;
                    break;
                case CMD_READ_INFO:
                {
                    char str[2];
                    sprintf(str, "%02x%02x", buffer[6], buffer[7]);
                    int value = atoi(str);
                    if (_meterCallback)
                        _meterCallback(buffer[5], value);
                    break;
                }
                case CMD_COMMAND_OK:
                    // FE FE E0 A4 FB FD
                    break;
                default:
                    knownCommand = false;
                }

                if (updateFrequency)
                {
                    if (_frequencyCallback)
                    {
                        _frequencyCallback(
                            _frequency,
                            _modulation,
                            _filter,
                            _txState);
                    }
                }
            }
        }

#ifdef DEBUG
        if (!knownCommand)
        {
            DBG("Unknown command: \n");
            dumpBuffer(buffer, size);
        }
#endif
    }
}

bool ICOM::initializeRig()
{
    DBG(">> CMD_READ_FREQ\n");
    // First request is sent to the broadcast address (0x00), once the radioAddress is received, next request are sent
    sendCodeRequest(CMD_READ_FREQ);
    uint8_t timeout = _readtimeout;
    while (_radioAddress == 0x00)
    {
        delay(100);
        if (--timeout == 0)
            break;
    }
    if (_radioAddress == 0x00)
    {
        return false;
    }
    else
    {
        DBG(">> CMD_READ_MODE + CMD_TRANSMIT_STATE\n");
        uint8_t req[] = {
            START_BYTE, START_BYTE, _radioAddress, CONTROLLER_ADDRESS, CMD_READ_MODE, STOP_BYTE,                       // Read currenet mode
            START_BYTE, START_BYTE, _radioAddress, CONTROLLER_ADDRESS, CMD_TRANSMIT_STATE, 0x00, 0x00, 0x01, STOP_BYTE // Subscribe for TX state changes
        };

        sendRawRequest(req, sizeof(req));
        return true;
    }
}

void ICOM::loop()
{
    if (_radioAddress != 0x00 && millis() > _timer1)
    {
        _timer1 = millis() + METER_POOL_INTERVAL;

        if (_txState) // while in TX, read POWER and SWR levels
        {
            uint8_t req[] = {
                START_BYTE, START_BYTE, _radioAddress, CONTROLLER_ADDRESS, CMD_READ_INFO, CMD_SUB_POWER, STOP_BYTE,
                START_BYTE, START_BYTE, _radioAddress, CONTROLLER_ADDRESS, CMD_READ_INFO, CMD_SUB_SWR, STOP_BYTE};
            sendRawRequest(req, sizeof(req));
        }
        else // while in RX, read S-Meter LEVEL
        {
            uint8_t req[] = {
                START_BYTE, START_BYTE, _radioAddress, CONTROLLER_ADDRESS, CMD_READ_INFO, CMD_SUB_S_METER, STOP_BYTE};
            sendRawRequest(req, sizeof(req));
        }
    }
}
