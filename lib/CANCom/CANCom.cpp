// CANCom.cpp
#include "CANCom.hpp"

CANCom::CANCom(uint8_t cs) : mcp(cs), csPin(cs) {}

bool CANCom::begin()
{
    // MCP_ANY = filtre sur tous les ID, CAN_500KBPS = vitesse du bus
    if (mcp.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    {
        Serial.println("[CAN] MCP2515 ready");
        return true;
    }
    else
    {
        Serial.println("[CAN] MCP2515 init failed");
        return false;
    }
}

bool CANCom::send(const CANFrame &frame)
{
    return mcp.sendMsgBuf(
               frame.id,
               0,
               frame.dlc,
               (uint8_t *)frame.data // cast
               ) == CAN_OK;
}

bool CANCom::receive(CANFrame &frame, uint32_t timeoutMs)
{
    unsigned long id;
    uint8_t len;
    uint8_t buf[8];
    unsigned long start = millis();

    while (millis() - start < timeoutMs)
    {
        if (mcp.readMsgBuf(&id, &len, buf) == CAN_OK)
        {
            frame.id = id;
            frame.dlc = len;
            memcpy(frame.data, buf, len);
            return true;
        }
    }

    return false; // timeout
}