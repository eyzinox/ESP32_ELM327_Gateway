#ifndef CANBUS_HPP
#define CANBUS_HPP

#include "CANFrame.hpp"

class CANBus {
public:
    virtual bool begin() = 0;
    virtual bool send(const CANFrame& frame) = 0;
    virtual bool receive(CANFrame& frame, uint32_t timeoutMs) = 0;
};

#endif // CANBUS_HPP