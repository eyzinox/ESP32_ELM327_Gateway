#include <Arduino.h>
#include "BLECom.hpp"

BLECom bleCom;

void setup()
{
  bleCom.begin();
}

void loop()
{
  // Tout est géré par BLECom
}