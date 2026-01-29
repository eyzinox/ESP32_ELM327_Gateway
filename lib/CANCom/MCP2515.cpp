#include "MCP2515.hpp"
#include <Arduino.h>

bool MCP2515CAN::begin() {
    Serial.println("[CAN] MCP2515 init (stub)");
    return true;
}

bool MCP2515CAN::send(const CANFrame& frame) {
    Serial.printf("[CAN] TX ID:%03lX DLC:%d\n", frame.id, frame.dlc);
    return true;
}

bool MCP2515CAN::receive(CANFrame& frame, uint32_t timeoutMs) {
    // stub : renvoie une trame fictive pour tester
    frame.id = 0x7E8;
    frame.dlc = 5;
    frame.data[0] = 0x04;
    frame.data[1] = 0x41;
    frame.data[2] = 0x0C;
    frame.data[3] = 0x0B;
    frame.data[4] = 0xB8;
    return true;
}
