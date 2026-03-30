#include <Arduino.h>
#include <mcp2515.h>
#include <SPI.h>

#include "com_AT.h"

#define DEBUG 1
#define EmulateMode 0 // 0: réel, 1: émulation

MCP2515 *mcp2515 = nullptr;

String sendOBDRequest(uint8_t mode, uint8_t pid);

void debugCAN(const char *label, const struct can_frame &f)
{
    if (!DEBUG)
        return;
    Serial.printf("[%s] ID=0x%03X DLC=%d | ", label, f.can_id, f.can_dlc);
    for (int i = 0; i < f.can_dlc; i++)
        Serial.printf("%02X ", f.data[i]);
    Serial.println();
}

void setupOBD(int CS_PIN, CAN_SPEED speed, CAN_CLOCK clock)
{
    mcp2515 = new MCP2515(CS_PIN);
    mcp2515->reset();
    mcp2515->setBitrate(speed, clock);
    mcp2515->setConfigMode();
    // Configuration des filtres pour OBD (0x7E8-0x7EF) - CAN 11-bit
    // Masque = 0x7F8 ignore les 3 bits LSB (bits 2:0)
    mcp2515->setFilterMask(MCP2515::MASK0, false, 0x7F8); // Pour RXF0-RXF2, 11-bit
    mcp2515->setFilterMask(MCP2515::MASK1, false, 0x7F8); // Pour RXF3-RXF5, 11-bit

    // Filtres : tous pointent vers 0x7E8 (accepte 0x7E8-0x7EF)
    mcp2515->setFilter(MCP2515::RXF0, false, 0x7E8);
    mcp2515->setFilter(MCP2515::RXF1, false, 0x7E8);
    mcp2515->setFilter(MCP2515::RXF2, false, 0x7E8);
    mcp2515->setFilter(MCP2515::RXF3, false, 0x7E8);
    mcp2515->setFilter(MCP2515::RXF4, false, 0x7E8);
    mcp2515->setFilter(MCP2515::RXF5, false, 0x7E8);

    mcp2515->setNormalMode();
}

bool isCommandOBD(String cmd)
{
    uint8_t mode = (uint8_t)strtol(cmd.substring(0, 2).c_str(), nullptr, 16);
    return (mode >= 1 && mode <= 9) || (mode >= 0xA && mode <= 0xE);
}

String processCommandOBD(String cmd)
{
    String response = "";

    // Détermination du mode
    uint8_t mode = (uint8_t)strtol(cmd.substring(0, 2).c_str(), nullptr, 16);
    uint8_t pid = 0x00;

    switch (mode)
    {
    case 0x01:
        pid = (uint8_t)strtol(cmd.substring(2, 4).c_str(), nullptr, 16);
        debugCAN("Request process", (struct can_frame){.can_id = Config.requestHeader, .can_dlc = 8, .data = {0x02, mode, pid, 0, 0, 0, 0, 0}});
        response = sendOBDRequest(mode, pid);
        break;
    case 0x03:
    case 0x07:
    case 0x09:
        debugCAN("Request", (struct can_frame){.can_id = Config.requestHeader, .can_dlc = 8, .data = {0x01, mode, 0, 0, 0, 0, 0, 0}});
        break;
    default:
        break;
    }
    return response;
}

String sendOBDRequest(uint8_t mode, uint8_t pid)
{
    String response = "";
    // Flush CAN : vide le buffer de réception pour éviter de lire une ancienne réponse
    struct can_frame tmp;
    while (mcp2515->readMessage(&tmp) == MCP2515::ERROR_OK)
        ;

    // Envoi requête
    struct can_frame tx;
    tx.can_id = Config.requestHeader;
    tx.can_dlc = 8;

    if (mode == 0x01)
    {
        tx.data[0] = 0x02; // Nb (mode + pid)
        tx.data[1] = mode;
        tx.data[2] = pid;
    }
    else if (mode == 0x03) // lire dtc
    {
        tx.data[0] = 0x01; // Nb (mode)
        tx.data[1] = mode;
    }
    else if (mode == 0x04) // effacer dtc
    {
        tx.data[0] = 0x01; // Nb (mode)
        tx.data[1] = mode;
    }

    for (int i = 3; i < 8; i++)
        tx.data[i] = 0;

    debugCAN("Request send", tx);
    if (mcp2515->sendMessage(&tx) != MCP2515::ERROR_OK)
    {
        response = "NO DATA" + String((Config.linefeeds) ? "\r\n>" : "\r\r>");
        return response;
    }

    // Reception de la réponse
    struct can_frame rx;

    while (millis() < Config.timeout)
    {
        if (mcp2515->readMessage(&rx) == MCP2515::ERROR_OK)
        {
            debugCAN("Response", rx);
            if (rx.can_id >= 0x7E8 && rx.can_id <= 0x7EF)
            {
                if (rx.data[1] == 0x7F) // NACK
                {
                    response = "NO DATA" + String((Config.linefeeds) ? "\r\n>" : "\r\r>");
                    return response;
                }
                else if (rx.data[1] == (mode + 0x40)) // ACK
                {
                    uint8_t len = rx.data[0];

                    // Format réponse ELM
                    char resp[64];

                    // Formatage avec ou sans espaces
                    (Config.spaces) ? snprintf(resp, sizeof(resp), "%03X %02X %02X", rx.can_id, len, rx.data[1]) 
                        : snprintf(resp, sizeof(resp), "%03X%02X%02X", rx.can_id, len, rx.data[1]);

                    for (int i = 0; i < (len - 2); i++)
                    {
                        char tmp[8];

                        (Config.spaces) ? snprintf(tmp, sizeof(tmp), " %02X", rx.data[3 + i]) 
                            : snprintf(tmp, sizeof(tmp), "%02X", rx.data[3 + i]);
                        strcat(resp, tmp);
                    }

                    strcat(resp, (Config.linefeeds) ? "\r\n>" : "\r\r>");
                    return String(resp);
                }
            }
        }
    }

    response = "NO DATA" + String((Config.linefeeds) ? "\r\n>" : "\r\r>");
    return response;
}