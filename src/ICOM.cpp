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

void ICOM::onData(const uint8_t *buffer, size_t size)
{
    uint16_t offset = 0;
    for (uint8_t n = 0; n < size; n++)
    {
        if (buffer[n] == STOP_BYTE || buffer[n] == 0xFF)
        {
            uint8_t size = n - offset + 1;
            if (size <= BT_BUFFER_SIZE)
            {
                memcpy(_readBuffer, buffer + offset, size);
                offset += size;
                handleNextMessage(_readBuffer);
            }
        }
    }
}

void ICOM::begin(String bluetoothName)
{
    bt.begin(bluetoothName);

    //wrapper for "C-styled" callbacks. The onData event is already ported to C++ std::bind paradigm
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
        if (_clientDisconnectedCallback)
        {
            _clientDisconnectedCallback();
        }
        break;
    case ESP_SPP_DATA_IND_EVT: // Data Received: Useless as it dont flush internal buffer
        //readBluetoothBuffer();
        //Serial.printf("BT Data Received: %d\n", param->data_ind.len);
        break;
    case ESP_SPP_WRITE_EVT: // Write complete

        break;
    default:
#ifdef DEBUG
        Serial.printf("BT Event: %d\n", event);
#endif
    }
}

// uint8_t ICOM::readBluetoothBuffer()
// {

//     uint8_t byte;
//     uint8_t counter = 0;
//     uint32_t ed = _readTimeout;

//     while (true)
//     {
//         while (!bt.available())
//         {
//             delay(10);
//             if (--ed == 0)
//                 return 0;
//         }

//         ed = _readTimeout;
//         byte = bt.read();

//         _readBuffer[counter++] = byte;

//         if (byte == STOP_BYTE || byte == 0xFF)
//         {
//             handleNextMessage(_readBuffer);
//             counter = 0;

//             if (!bt.available()) //reached end of stream, stop reading
//                 return counter;
//         }

//         if (counter >= BT_BUFFER_SIZE)
//             return 0;
//     }
//     return counter;
// }

// void ICOM::waitForResponse()
// {
//     uint32_t ed = _readTimeout;
//     while (_pendingResponse)
//     {
//         delay(100);
//         if (--ed == 0)
//             break;
//     }
// }

void ICOM::dumpBuffer()
{
    Serial.print("Buffer: ");
    for (uint8_t i = 0; i < BT_BUFFER_SIZE; i++)
    {
        Serial.printf("%02X ", _readBuffer[i]);
        if (_readBuffer[i] == STOP_BYTE || _readBuffer[i] == 0xFF)
            break;
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
    // uint16_t timeout = _readTimeout;
    // while (!bt.availableForWrite())
    // {
    //     delay(10);
    //     if (--timeout == 0)
    //         break;
    // }
    bt.write(request, size);
    bt.flush();
}

void ICOM::handleNextMessage(uint8_t *buffer)
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

    if (buffer[0] == START_BYTE && buffer[1] == START_BYTE)
    {
        //if (buffer[3] == radio_address)
        //{

        if (buffer[2] == CONTROLLER_ADDRESS || buffer[2] == BROADCAST_ADDRESS)
        {
            switch (buffer[4])
            {
            case CMD_TRANS_FREQ:
            case CMD_READ_FREQ:
                //Serial.println("READ_FREQ");
                _frequency = 0;
                for (uint8_t i = 0; i < 5; i++)
                {
                    if (buffer[9 - i] == 0xFD)
                        continue; //spike
                    _frequency += (buffer[9 - i] >> 4) * decMulti[i * 2];
                    _frequency += (buffer[9 - i] & 0x0F) * decMulti[i * 2 + 1];
                }
                break;
            case CMD_TRANS_MODE:
            case CMD_READ_MODE:
                //Serial.println("READ_MODE");
                _modulation = buffer[5]; //FE FE E0 42 04 <00 01> FD
                _filter = buffer[6];     //01 - Wide, 02 - Medium, 03 - Narrow
                break;
            case CMD_TRANSMIT_STATE:
                //Serial.println("TRANSMIT_STATE");
                _txState = buffer[7] == 0x01;
                break;
            case CMD_COMMAND_OK:
                //FE FE E0 A4 FB FD
                break;
            default:
                knownCommand = false;
            }

            if (knownCommand)
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
        //}

#ifdef DEBUG
        if (!knownCommand)
            dumpBuffer();
#endif
    }
}

void ICOM::initializeRig()
{
#ifdef DEBUG
    Serial.println(">> CMD_READ_MODE");
#endif
    // Read currenet mode
    sendCodeRequest(CMD_READ_MODE);

#ifdef DEBUG
    Serial.println(">> CMD_TRANSMIT_STATE");
#endif
    //Subscribe for TX state changes
    uint8_t req[] = {START_BYTE, START_BYTE, _radioAddress, CONTROLLER_ADDRESS, CMD_TRANSMIT_STATE, 0x00, 0x00, 0x01, STOP_BYTE};
    sendRawRequest(req, sizeof(req));

#ifdef DEBUG
    Serial.println(">> CMD_READ_FREQ");
#endif
    // Read frequency for the first time
    sendCodeRequest(CMD_READ_FREQ);
}
