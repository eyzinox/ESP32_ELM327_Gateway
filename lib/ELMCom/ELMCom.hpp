#ifndef ELMCOM_HPP
#define ELMCOM_HPP

#include "CANCom.hpp"
#include "CANFrame.hpp"

#include <stddef.h>

#define MAX_RESP_LEN 64

class ELMCom
{
private:
    CANCom *can = nullptr;

    bool isATCommand(const char *cmd);
    const char *handleATCommand(const char *cmd, size_t &len);
    const char *handleOBDCommand(const char *cmd, size_t &len);

public:
    void attachCAN(CANCom *bus);
    const char *processCommand(const char *cmd, size_t &len);
};

#endif // ELMCOM_HPP