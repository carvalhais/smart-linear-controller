#include <Arduino.h>
#include "Types.h"
#include "Controller.h"

Controller ctl;

void setup()
{
  Serial.begin(115200);
  ctl.begin();
}

void loop(void)
{
  ctl.loop();
}
