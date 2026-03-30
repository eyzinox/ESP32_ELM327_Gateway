#ifndef COM_OBD_H
#define COM_OBD_H

#include <mcp2515.h>

extern MCP2515* mcp2515;

void setupOBD(int CS_PIN, CAN_SPEED speed, CAN_CLOCK clock);
bool isCommandOBD(String cmd);
String processCommandOBD(String cmd);
String sendOBDRequest(uint8_t mode, uint8_t pid);

#endif // COM_OBD_H