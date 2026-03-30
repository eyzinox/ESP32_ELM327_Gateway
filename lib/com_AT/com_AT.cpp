#include <Arduino.h>
#include "com_AT.h"
    
ELM327Config Config;  // Définition de la variable globale

bool isCommandAT(String cmd)
{
    if (cmd.startsWith("AT")) { return true; }
    else if (cmd == "STI" || cmd == "VTI") { return true; }
    else { return false; }
}

String processCommandAT(String cmd)
{
    String response = "";
    if (cmd == "ATZ" || cmd == "ATI") { response = "ELM327 v1.5"; }
    else if (cmd == "STI") { response = "ELM327 v1.5"; }
    else if (cmd == "VTI") { response = "ELM327 v1.5"; }
    else if (cmd == "ATE0") { Config.echo = false; response = "OK"; }
    else if (cmd == "ATE1") { Config.echo = true; response = "OK"; }

    else if (cmd == "ATH0") { Config.headers = false; response = "OK"; }
    else if (cmd == "ATH1") { Config.headers = true; response = "OK"; }

    else if (cmd == "ATS0") { Config.spaces = false; response = "OK"; }
    else if (cmd == "ATS1") { Config.spaces = true; response = "OK"; }

    else if (cmd == "ATL0") { Config.linefeeds = false; response = "OK"; }
    else if (cmd == "ATL1") { Config.linefeeds = true; response = "OK"; }

    else if (cmd == "ATAL") { Config.autoProtocol = true; response = "OK"; }
    else if (cmd == "ATAT1") { Config.autoProtocol = false; response = "OK"; }

    //else if (cmd == "ATD0") { Config.DTC = false; response = "OK"; }
    //else if (cmd == "ATD1") { Config.DTC = true; response = "OK"; }

    else if (cmd.startsWith("ATSH")) { Config.requestHeader = (uint16_t)strtol(cmd.substring(4).c_str(), nullptr, 16); response = "OK"; }
    else if (cmd.startsWith("ATSP")) { response = "OK"; }
    else if (cmd.startsWith("ATST")) { Config.timeout = (uint16_t)strtol(cmd.substring(4).c_str(), nullptr, 16) * 4; response = "OK"; }

    else if (cmd == "ATDP") { response = "ISO 15765-4 (CAN 11/500)"; }
    else if (cmd == "ATDPN") { response = "A6"; }
    else if (cmd == "ATRV") { response = "13.8V"; }
    else { response = "?"; }

    (Config.linefeeds) ? response += "\r\n>" : response += "\r\r>";
    return response;
}