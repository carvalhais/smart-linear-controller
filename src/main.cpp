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

  auto cbMeter = std::bind(&Controller::onMeterUpdated,
                           &ctl,
                           std::placeholders::_1,
                           std::placeholders::_2);

  icom.onConnectedCallback(cbConnected);
  icom.onFrequencyCallback(cbFrequency);
  icom.onDisconnectedCallback(cbDisconnected);
  icom.onMeterCallback(cbMeter);

  auto cbInputSwr = std::bind(&Controller::onInputSwr,
                              &ctl,
                              std::placeholders::_1,
                              std::placeholders::_2);
  auto cbOutputSwr = std::bind(&Controller::onOutputSwr,
                               &ctl,
                               std::placeholders::_1,
                               std::placeholders::_2);
  auto cbButtons = std::bind(&Controller::onButtonPressed,
                             &ctl,
                             std::placeholders::_1,
                             std::placeholders::_2);

  hal.onInputSwrCallback(cbInputSwr);
  hal.onOutputSwrCallback(cbOutputSwr);
  hal.onButtonPressedCallback(cbButtons);

  auto cbAmp = std::bind(&HardwareLayer::onAmplifierChanged,
                         &hal,
                         std::placeholders::_1);

  auto cbLpf = std::bind(&HardwareLayer::onLowPassFilterChanged,
                         &hal,
                         std::placeholders::_1);

  auto cbTx = std::bind(&HardwareLayer::onTransmitChanged,
                        &hal,
                        std::placeholders::_1);

  auto cbPsu = std::bind(&HardwareLayer::onPowerSupplyChanged,
                         &hal,
                         std::placeholders::_1);

  ctl.onAmplifierCallback(cbAmp);
  ctl.onLowPassFilterCallback(cbLpf);
  ctl.onTransmitCallback(cbTx);
  ctl.onPowerSupplyCallback(cbPsu);
  icom.begin(BT_NAME);

  ctl.begin(&icom);
  hal.begin();

#ifdef M5STACK
  ledcSetup(BLK_PWM_CHANNEL, 50000, 8);
  ledcAttachPin(TFT_BL, BLK_PWM_CHANNEL);
  ledcWrite(BLK_PWM_CHANNEL, 50);
#endif
}

void loop(void)
{
  ctl.loop();
  hal.loop();
  icom.loop();
}
