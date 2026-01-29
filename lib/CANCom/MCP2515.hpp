#ifndef CANCOM_MCP2515_HPP
#define CANCOM_MCP2515_HPP

#include "CANBus.hpp"

class MCP2515CAN : public CANBus {
public:
    bool begin() override;
    bool send(const CANFrame& frame) override;
    bool receive(CANFrame& frame, uint32_t timeoutMs) override;
};

#endif 