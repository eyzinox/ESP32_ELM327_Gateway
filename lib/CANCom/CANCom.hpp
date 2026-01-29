#ifndef CANCOM_HPP
#define CANCOM_HPP

#include <Arduino.h>
#include <mcp_can.h>
#include "CANFrame.hpp"

class CANCom {
private:
    MCP_CAN mcp;
    uint8_t csPin;

public:
    CANCom(uint8_t cs);        // constructeur avec pin CS
    bool begin();              // initialisation du MCP2515
    bool send(const CANFrame& frame);
    bool receive(CANFrame& frame, uint32_t timeoutMs = 100); // timeout en ms
};

#endif // CANCOM_HPP