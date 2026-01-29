#ifndef OBDCOM_HPP
#define OBDCOM_HPP

#include <Arduino.h>

class OBDCom
{
public:
    OBDCom() {}
    ~OBDCom() {}

    // Retourne un pointeur vers la réponse, et met à jour la longueur
    const char *processCommand(const char *cmd, size_t &respLen)
    {
        const char *response = nullptr;
        if (strcmp(cmd, "ATZ") == 0 || strcmp(cmd, "ATI") == 0)
        {
            response = "ELM327 v1.5\r>";
            // respLen = strlen(response);
            respLen = 12;
        }
        else if (strcmp(cmd, "ATE0") == 0 || strcmp(cmd, "ATL0") == 0 ||
                 strcmp(cmd, "ATS0") == 0 || strcmp(cmd, "ATH1") == 0 ||
                 strcmp(cmd, "ATSP6") == 0 || strcmp(cmd, "ATCAF0") == 0)
        {
            response = "OK\r>";
            // respLen = strlen(response);
            respLen = 4;
        }
        else if (strcmp(cmd, "ATDPN") == 0)
        {
            response = "A6\r>";
            respLen = 4;
        }
        else if (strcmp(cmd, "ATDP") == 0)
        {
            response = "ISO 15765-4 (CAN 11/500)\r>";
            respLen = 26;
        }
        else if (strcmp(cmd, "ATRV") == 0)
        {
            response = "13.8V\r>";
            respLen = 7;
        }
        else if (strcmp(cmd, "0100") == 0)
        {
            response = "7E8 06 41 00 FF FF FF FF\r>";
            respLen = strlen(response);
        }
        else if (strcmp(cmd, "010C") == 0)
        {
            response = "7E8 04 41 0C 0B B8\r>";
            respLen = strlen(response);
        }
        else if (strcmp(cmd, "010D") == 0)
        {
            response = "7E8 03 41 0D 64\r>";
            respLen = strlen(response);
        }
        else
        {
            response = "?\r>";
            respLen = strlen(response);
        }
        return response;
    }
};

#endif
