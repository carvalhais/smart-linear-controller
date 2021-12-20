#include <Arduino.h>

#include "Types.h"
#include "Controller.h"
#include "ICOM.h"
#include "HardwareLayer.h"

ICOM icom;
Controller ctl;
HardwareLayer hal;

void setup()
{
  Serial.begin(115200);

  auto cbConnected = std::bind(&Controller::onClientConnected,
                               &ctl,
                               std::placeholders::_1);
  auto cbFrequency = std::bind(&Controller::onFrequencyChanged,
                               &ctl,
                               std::placeholders::_1,
                               std::placeholders::_2,
                               std::placeholders::_3,
                               std::placeholders::_4);
  auto cbDisconnected = std::bind(&Controller::onClientDisconnected,
                                  &ctl);

  icom.onConnectedCallback(cbConnected);
  icom.onFrequencyCallback(cbFrequency);
  icom.onDisconnectedCallback(cbDisconnected);

  auto cbInputSwr = std::bind(&Controller::onInputSwr,
                              &ctl,
                              std::placeholders::_1,
                              std::placeholders::_2);
  auto cbOutputSwr = std::bind(&Controller::onOutputSwr,
                               &ctl,
                               std::placeholders::_1,
                               std::placeholders::_2);

  hal.onInputSwrCallback(cbInputSwr);
  hal.onOutputSwrCallback(cbOutputSwr);

  auto cbAmp = std::bind(&HardwareLayer::onAmplifierChanged,
                         &hal,
                         std::placeholders::_1);

  auto cbLpf = std::bind(&HardwareLayer::onLowPassFilterChanged,
                         &hal,
                         std::placeholders::_1);

  auto cbTx = std::bind(&HardwareLayer::onTransmitChanged,
                         &hal,
                         std::placeholders::_1);                         

  ctl.onAmplifierCallback(cbAmp);
  ctl.onLowPassFilterCallback(cbLpf);
  ctl.onTransmitCallback(cbTx);

  icom.begin(BT_NAME);

  ctl.begin(&icom);
  hal.begin();
}

void loop(void)
{
  ctl.loop();
  hal.loop();
}
