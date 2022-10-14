#include "CAN.h"

CAN::CAN()
{
}
bool CAN::begin(uint8_t pinTx, uint8_t pinRx)
{
#ifdef ESP8266
    _can.reset();
    MCP2515::ERROR error = _can.setBitrate(CAN_125KBPS, MCP_8MHZ);
    if (error != MCP2515::ERROR_OK)
    {
        DBG("CAN (MCP2515): Failed to initialize CAN driver\n");
        return true;
    }
    else
    {
        _can.setNormalMode();
        DBG("CAN (MCP2515): Setup complete\n");
        return false;
    }
#endif

#ifdef ESP32ACAN
    ACAN_ESP32_Settings settings(125E3);
    settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode;
    settings.mRxPin = (gpio_num_t)PIN_CAN_RX;
    settings.mTxPin = (gpio_num_t)PIN_CAN_TX;
    const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::acceptExtendedFrames();
    const uint32_t errorCode = ACAN_ESP32::can.begin(settings, filter);
    if (errorCode == 0)
    {
        DBG("CAN (ACAN): Setup complete\n");
        return true;
    }
    else
    {
        DBG("CAN (ACAN): Failed to initialize CAN driver\n");
        return false;
    }
#endif

#ifdef ESP32TWAI
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)PIN_CAN_TX, (gpio_num_t)PIN_CAN_RX, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    // Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        // Start TWAI driver
        if (twai_start() == ESP_OK)
        {
            DBG("CAN (TWAI): Setup complete\n");
            return true;
        }
    }
    DBG("CAN (TWAI): Failed to initialize CAN driver\n");
    return false;

#endif
}

void CAN::end()
{
#ifdef ESP32TWAI
    // Stop the TWAI driver
    if (twai_stop() == ESP_OK)
    {
        twai_driver_uninstall();
    }
#endif
}
bool CAN::receive(PowerSupplyMessage *message)
{

#ifdef ESP8266
    struct can_frame frame;
    if (_can.readMessage(&frame) == MCP2515::ERROR_OK)
    {
        if (!(frame.can_id & CAN_RTR_FLAG))
        {
            message->id = frame.can_id;
            message->len = frame.can_dlc;
            memcpy(message->data, frame.data, frame.can_dlc);
            return true;
        }
    }
    return false;

#endif

#ifdef ESP32ACAN
    CANMessage frame;
    if (ACAN_ESP32::can.receive(frame))
    {
        if (!frame.rtr)
        {
            message->id = frame.id;
            message->len = frame.len;
            memcpy(message->data, frame.data, frame.len);
            return true;
        }
    }
    return false;
#endif
#ifdef ESP32TWAI
    twai_status_info_t status;
    twai_get_status_info(&status);
    if (status.msgs_to_rx > 0)
    {
        twai_message_t frame;
        if (twai_receive(&frame, pdMS_TO_TICKS(1000)) == ESP_OK)
        {
            if (!(frame.rtr))
            {
                message->id = frame.identifier;
                message->len = frame.data_length_code;
                memcpy(message->data, frame.data, frame.data_length_code);
                return true;
            }
        }
    }
    return false;
#endif
}
bool CAN::send(uint32_t id, uint8_t len, uint8_t data[])
{
#ifdef ESP8266
    struct can_frame frame;
    frame.can_id = id | CAN_EFF_FLAG;
    frame.can_dlc = len;
    memcpy(frame.data, data, len);
    return _can.sendMessage(&frame) == MCP2515::ERROR_OK;
#endif

#ifdef ESP32ACAN
    CANMessage frame;
    frame.id = id;
    frame.len = len;
    frame.ext = true;
    memcpy(frame.data, data, len);
    return ACAN_ESP32::can.tryToSend(frame);
#endif
#ifdef ESP32TWAI
    // Configure message to transmit
    twai_message_t message;
    message.identifier = id;
    message.extd = 1;
    message.rtr = 0;
    message.ss = 0;
    message.self = 0;
    message.data_length_code = len;
    memcpy(message.data, data, len);
    // Queue message for transmission
    return twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK;
#endif
}