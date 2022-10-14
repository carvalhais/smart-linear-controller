#include <Arduino.h>
#include "Types.h"
#include "Controller.h"

Controller ctl;
timer_t last = millis();
uint8_t count = 0;

void setup()
{
  Serial.begin(115200);
  ctl.begin();
}

void loop(void)
{
  ctl.loop();
  // if (count > 30)
  // {
  //   DBG("MAIN: Loop interval: %dms [Core %d]\n", millis() - last, xPortGetCoreID());
  // }
  // count++;
  // last = millis();
}
