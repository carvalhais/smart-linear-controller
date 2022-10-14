#ifndef _CAN_CLASS_
#define _CAN_CLASS_

#include <Arduino.h>
#include <defines.h>
#include <memory>
#include <CAN/CANTypes.h>

#ifdef ESP8266
#include <mcp2515.h>
#endif

#ifdef ESP32ACAN
#include <ACAN_ESP32.h>
#endif

#ifdef ESP32TWAI
#include "driver/gpio.h"
#include "driver/twai.h"
#endif

class CAN
{

public:

    CAN();
    bool begin(uint8_t pinTx, uint8_t pinRx);
    void end();
    bool receive(PowerSupplyMessage *message);
    bool send(uint32_t id, uint8_t len, uint8_t data[]);

private:
#ifdef ESP8266
    MCP2515 _can = MCP2515(PIN_CAN_CS);
#endif
};
#endif
