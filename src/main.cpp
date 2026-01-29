#include <Arduino.h>
#include "BLECom.hpp"
#include "ELMCom.hpp"
#include "MCP2515.hpp"

BLECom ble;
ELMCom elm;
MCP2515CAN can;

void setup()
{
  Serial.begin(115200);
  can.begin();
  elm.attachCAN(&can);
  ble.attachELM(&elm);
  ble.begin();
}

void loop()
{
}
