#include <Arduino.h>
#include "CANCom.hpp"
#include "ELMCom.hpp"
#include "BLECom.hpp"

BLECom ble;
ELMCom elm;
CANCom can(5); // CS MCP2515 = GPIO5

void setup() {
    Serial.begin(115200);
    can.begin();
    elm.attachCAN(&can); // relie ELMCom à CANCom
    ble.attachELM(&elm); // relie BLECom à ELMCom
    ble.begin();
}

void loop() {
    // tout est géré par BLE callbacks
}
