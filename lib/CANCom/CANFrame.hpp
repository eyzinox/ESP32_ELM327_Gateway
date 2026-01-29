#ifndef CANFRAME_HPP
#define CANFRAME_HPP

#include <Arduino.h>

struct CANFrame {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
};

#endif // CANFRAME_HPP