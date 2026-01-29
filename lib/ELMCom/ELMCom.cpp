// ELMCom.cpp
#include "ELMCom.hpp"
#include <Arduino.h>
#include <cstring>

void ELMCom::attachCAN(CANCom *bus)
{
    can = bus;
}

bool ELMCom::isATCommand(const char *cmd)
{
    return (cmd[0] == 'A' && cmd[1] == 'T');
}

const char *ELMCom::handleATCommand(const char *cmd, size_t &len)
{
    static char resp[MAX_RESP_LEN];

    if (strcmp(cmd, "ATZ") == 0 || strcmp(cmd, "ATI") == 0)
    {
        strcpy(resp, "ELM327 v1.5\r>");
        len = 12;
    }
    else if (strcmp(cmd, "ATE0") == 0 || strcmp(cmd, "ATL0") == 0 ||
             strcmp(cmd, "ATS0") == 0 || strcmp(cmd, "ATH1") == 0 ||
             strcmp(cmd, "ATSP6") == 0 || strcmp(cmd, "ATCAF0") == 0)
    {
        strcpy(resp, "OK\r>");
        len = 4;
    }
    else if (strcmp(cmd, "ATDPN") == 0)
    {
        strcpy(resp, "A6\r>");
        len = 4;
    }
    else if (strcmp(cmd, "ATDP") == 0)
    {
        strcpy(resp, "ISO 15765-4 (CAN 11/500)\r>");
        len = 26;
    }
    else if (strcmp(cmd, "ATRV") == 0)
    {
        strcpy(resp, "13.8V\r>");
        len = 7;
    }
    else
    {
        strcpy(resp, "?\r>");
        len = 3;
    }

    return resp;
}

const char *ELMCom::handleOBDCommand(const char *cmd, size_t &len)
{
    static char resp[MAX_RESP_LEN];
    if (!can)
    {
        strcpy(resp, "NO CAN\r>");
        len = strlen(resp);
        return resp;
    }

    CANFrame frame{};

    if (strcmp(cmd, "0100") == 0)
    {
        // Requête CAN pour PID 0100
        frame.id = 0x7DF; // ID de requête
        frame.dlc = 8;
        frame.data[0] = 0x02; // nombre d'octets suivants
        frame.data[1] = 0x01; // mode 01
        frame.data[2] = 0x00; // PID 00
        frame.data[3] = 0x00; // padding
        frame.data[4] = 0x00;
        frame.data[5] = 0x00;
        frame.data[6] = 0x00;
        frame.data[7] = 0x00;

        can->send(frame);

        // Réponse simulée pour le BLE
        sprintf(resp, "7E8 06 41 00 FF FF FF FF\r>");
        len = strlen(resp);
        return resp;
    }
    else if (strcmp(cmd, "010C") == 0)
    {
        // Requête CAN pour PID 010C
        frame.id = 0x7DF;
        frame.dlc = 8;
        frame.data[0] = 0x02;
        frame.data[1] = 0x01;
        frame.data[2] = 0x0C;
        frame.data[3] = 0x00;
        frame.data[4] = 0x00;
        frame.data[5] = 0x00;
        frame.data[6] = 0x00;
        frame.data[7] = 0x00;

        can->send(frame);

        CANFrame responseFrame;
        if (can->receive(responseFrame, 100))
        {
            sprintf(resp, "%03lX %02X %02X %02X %02X %02X\r>",
                    responseFrame.id,
                    responseFrame.dlc,
                    responseFrame.data[1],
                    responseFrame.data[2],
                    responseFrame.data[3],
                    responseFrame.data[4]);
        }
        else
        {
            strcpy(resp, "NO DATA\r>");
        }

        len = strlen(resp);
        return resp;
    }
    else
    {
        strcpy(resp, "?\r>");
        len = strlen(resp);
        return resp;
    }
}

const char *ELMCom::processCommand(const char *cmd, size_t &len)
{
    if (isATCommand(cmd))
        return handleATCommand(cmd, len);
    else
        return handleOBDCommand(cmd, len);
}
