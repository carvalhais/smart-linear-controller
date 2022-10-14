#include "ATU1000.h"

ATU1000::ATU1000()
{
}

void ATU1000::begin()
{
    pinMode(ATU_RX_PIN, INPUT);
    Serial1.begin(9600, SERIAL_8N1, ATU_RX_PIN);
}

void ATU1000::end()
{
}

void ATU1000::loop()
{
    while (Serial1.available())
    {
        int c = Serial1.read();
        DBG("Received: %d %c\n", c, char(c));
    }
}