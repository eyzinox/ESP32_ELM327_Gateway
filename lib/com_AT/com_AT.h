#ifndef COM_AT_H
#define COM_AT_H

struct ELM327Config {
    bool echo = true;
    bool headers = true;
    bool spaces = true;
    bool linefeeds = false;
    bool autoProtocol = true;
    bool DTC = true;
    uint16_t requestHeader = 0x7DF;
    int timeout = 32;
};
extern ELM327Config Config;

bool isCommandAT(String cmd);
String processCommandAT(String cmd);

#endif // COM_AT_H