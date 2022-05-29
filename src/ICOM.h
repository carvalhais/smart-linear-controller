#ifndef _ICOM_CLASS_
#define _ICOM_CLASS_

#include "BluetoothSerial.h"
#include "Defines.h"
#include "Types.h"

#define BROADCAST_ADDRESS 0x00  // Broadcast address
#define CONTROLLER_ADDRESS 0xE0 // Controller address

#define START_BYTE 0xFE // Start byte
#define STOP_BYTE 0xFD  // Stop byte

#define CMD_TRANS_FREQ 0x00 // Transfers operating frequency data
#define CMD_TRANS_MODE 0x01 // Transfers operating mode data

#define CMD_READ_FREQ 0x03 // Read operating frequency data
#define CMD_READ_MODE 0x04 // Read operating mode data

#define CMD_WRITE_FREQ 0x05 // Write operating frequency data
#define CMD_WRITE_MODE 0x06 // Write operating mode data

#define CMD_READ_INFO 0x15   // Read varios meters (Signal, Power)
#define CMD_SUB_S_METER 0x02 // S-Meter reading
#define CMD_SUB_POWER 0x11 // POWER reading
#define CMD_SUB_SWR 0x12 // S-Meter reading

#define CMD_TRANSMIT_STATE 0x24 // Read TX Mode (On/Off)
#define CMD_COMMAND_OK 0xFB     // Last command was accepted

#define BT_BUFFER_SIZE 12

#define METER_POOL_INTERVAL 1000

// #define IF_PASSBAND_WIDTH_WIDE 0x01
// #define IF_PASSBAND_WIDTH_MEDIUM 0x02
// #define IF_PASSBAND_WIDTH_NARROW 0x03

// #define MODE_TYPE_LSB 0x00
// #define MODE_TYPE_USB 0x01
// #define MODE_TYPE_AM 0x02
// #define MODE_TYPE_CW 0x03
// #define MODE_TYPE_RTTY 0x04
// #define MODE_TYPE_FM 0x05

typedef std::function<void(uint32_t frequency, uint8_t modulation, uint8_t filter, bool txState)> FrequencyCb;
typedef std::function<void(uint8_t type, uint8_t value)> MeterCb;
typedef std::function<void(uint8_t[6])> ClientConnectedCb;
typedef std::function<void()> ClientDisconnectedCb;

class ICOM
{
public:
    ICOM();
    void begin(String bluetoothName);
    void onFrequencyCallback(FrequencyCb cb);
    void onMeterCallback(MeterCb cb);
    void onConnectedCallback(ClientConnectedCb cb);
    void onDisconnectedCallback(ClientDisconnectedCb cb);
    bool initializeRig();
    void loop();
    static ICOM *instance;

private:
    void onData(const uint8_t *buffer, size_t size);
    void eventCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    void dumpBuffer(uint8_t *buffer, uint8_t size);
    void sendCodeRequest(uint8_t requestCode);
    void sendRawRequest(uint8_t request[], uint8_t size);
    void handleNextMessage(uint8_t *buffer, uint8_t size);
    void processCatMessages();
    FrequencyCb _frequencyCallback;
    MeterCb _meterCallback;
    ClientConnectedCb _clientConnectedCallback;
    ClientDisconnectedCb _clientDisconnectedCallback;
    BluetoothSerial bt;
    const uint32_t decMulti[10] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
    uint8_t _radioAddress = 0x00;
    uint8_t _readtimeout = 10;
    uint32_t _frequency;
    uint8_t _modulation = 0;
    uint8_t _filter = 0;
    uint16_t _readTimeout = 100; //*100ms
    bool _txState = false;
    timer_t _timer1;
};

#endif