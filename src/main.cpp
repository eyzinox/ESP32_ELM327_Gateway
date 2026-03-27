#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <mcp2515.h>
#include <SPI.h>

// ─── BLE ─────────────────────────────────────────────────────
#define DEVICE_NAME "OBD_Logger"
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID_RX "12345678-1234-1234-1234-1234567890ac"
#define CHAR_UUID_TX "12345678-1234-1234-1234-1234567890ad"

// ─── CAN ─────────────────────────────────────────────────────
#define CS_PIN 5
#define CAN_SPEED CAN_500KBPS
#define CAN_CLOCK MCP_8MHZ

#define OBD_REQUEST 0x7DF
#define RESPONSE_MIN 0x7E8
#define RESPONSE_MAX 0x7EF

#define CAN_TIMEOUT_MS 200

BLECharacteristic *pTxCharacteristic = nullptr;
MCP2515 mcp2515(CS_PIN);

// ─── Debug CAN ───────────────────────────────────────────────
void debugCAN(const char *label, const struct can_frame &f)
{
    Serial.printf("[%s] ID=0x%03X DLC=%d | ", label, f.can_id, f.can_dlc);
    for (int i = 0; i < f.can_dlc; i++)
        Serial.printf("%02X ", f.data[i]);
    Serial.println();
}

// ─── Envoi BLE ───────────────────────────────────────────────
void sendResponse(const char *resp)
{
    if (pTxCharacteristic)
    {
        pTxCharacteristic->setValue((uint8_t *)resp, strlen(resp));
        pTxCharacteristic->notify();
    }
}

// ─── Traitement commandes ─────────────────────────────────────
void processCommand(String cmd)
{
    cmd.trim();
    cmd.toUpperCase();

    if (cmd.startsWith("01") && cmd.length() == 5 && cmd.endsWith("1"))
        cmd = cmd.substring(0, 4);

    Serial.printf("[CMD] %s\n", cmd.c_str());

    // ─── AT commands ─────────────────────────────
    if (cmd == "ATZ" || cmd == "ATI" || cmd == "STI" || cmd == "VTI")
    {
        sendResponse("ELM327 v1.5\r\n>");
        return;
    }
    else if (cmd == "ATE0" || cmd == "ATH1" || cmd == "ATS0" || cmd == "ATM0" || cmd == "ATAL" || cmd == "ATAT1")
    {
        sendResponse("OK\r\n>");
        return;
    }
    else if (cmd == "ATDP")
    {
        sendResponse("ISO 15765-4 (CAN 11/500)\r\n>");
        return;
    }
    else if (cmd == "ATDPN")
    {
        sendResponse("A6\r\n>");
        return;
    }
    else if (cmd == "ATRV")
    {
        sendResponse("13.8V\r\n>");
        return;
    }
    else if (cmd.startsWith("AT"))
    {
        sendResponse("OK\r\n>");
        return;
    }

    // ─── MODE 01 ────────────────────────────────
    if (cmd.startsWith("01"))
    {
        uint8_t pid = (uint8_t)strtol(cmd.substring(2).c_str(), nullptr, 16);

        Serial.printf("[OBD] PID 0x%02X\n", pid);

        // Flush CAN
        struct can_frame tmp;
        while (mcp2515.readMessage(&tmp) == MCP2515::ERROR_OK)
            ;

        // ─── Envoi requête ───────────────────────
        struct can_frame tx;
        tx.can_id = OBD_REQUEST;
        tx.can_dlc = 8;

        tx.data[0] = 0x02;
        tx.data[1] = 0x01;
        tx.data[2] = pid;
        for (int i = 3; i < 8; i++)
            tx.data[i] = 0;

        debugCAN("TX", tx);

        if (mcp2515.sendMessage(&tx) != MCP2515::ERROR_OK)
        {
            sendResponse("NO DATA\r\n>");
            return;
        }

        delay(5); // important

        // ─── Attente réponse ─────────────────────
        struct can_frame rx;
        unsigned long start = millis();

        while (millis() - start < CAN_TIMEOUT_MS)
        {
            if (mcp2515.readMessage(&rx) != MCP2515::ERROR_OK)
                continue;

            debugCAN("RX", rx);

            if (rx.can_id < RESPONSE_MIN || rx.can_id > RESPONSE_MAX)
                continue;

            if (rx.data[1] == 0x7F)
            {
                sendResponse("NO DATA\r\n>");
                return;
            }

            if (rx.data[1] != 0x41 || rx.data[2] != pid)
                continue;

            uint8_t len = rx.data[0];

            // ─── Format réponse ELM ───────────────
            char resp[64];
            snprintf(resp, sizeof(resp), "%03X %02X 41 %02X",
                     rx.can_id, len, pid);

            for (int i = 0; i < (len - 2); i++)
            {
                char tmp[8];
                snprintf(tmp, sizeof(tmp), " %02X", rx.data[3 + i]);
                strcat(resp, tmp);
            }

            strcat(resp, "\r\n>");

            Serial.printf("[RESP] %s\n", resp);
            sendResponse(resp);
            return;
        }

        Serial.println("[OBD] Timeout");
        sendResponse("NO DATA\r\n>");
        return;
    }

    // ─── Inconnu ───────────────────────────────
    sendResponse("NO DATA\r\n>");
}

// ─── BLE callbacks ───────────────────────────────────────────
class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *) override
    {
        Serial.println("[BLE] Connecté");
    }

    void onDisconnect(BLEServer *s) override
    {
        Serial.println("[BLE] Déconnecté");
        s->getAdvertising()->start();
    }
};

class RxCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pChar) override
    {
        if (!pChar)
            return;
        String val = pChar->getValue();
        processCommand(val);
    }
};

// ─── Setup ───────────────────────────────────────────────────
void setup()
{
    Serial.begin(115200);
    Serial.println("ESP32 OBD BLE");

    // BLE
    BLEDevice::init(DEVICE_NAME);
    BLEServer *server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    BLEService *service = server->createService(SERVICE_UUID);

    pTxCharacteristic = service->createCharacteristic(
        CHAR_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *rxChar = service->createCharacteristic(
        CHAR_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
    rxChar->setCallbacks(new RxCallbacks());

    service->start();
    BLEDevice::getAdvertising()->start();

    Serial.println("BLE prêt");

    // CAN
    mcp2515.reset();
    mcp2515.setBitrate(CAN_SPEED, CAN_CLOCK);
    mcp2515.setConfigMode();

    // Configuration des filtres pour OBD (0x7E8-0x7EF) - CAN 11-bit
    // Masque = 0x7F8 ignore les 3 bits LSB (bits 2:0)
    mcp2515.setFilterMask(MCP2515::MASK0, false, 0x7F8);  // Pour RXF0-RXF2, 11-bit
    mcp2515.setFilterMask(MCP2515::MASK1, false, 0x7F8);  // Pour RXF3-RXF5, 11-bit

    // Filtres : tous pointent vers 0x7E8 (accepte 0x7E8-0x7EF)
    mcp2515.setFilter(MCP2515::RXF0, false, 0x7E8);
    mcp2515.setFilter(MCP2515::RXF1, false, 0x7E8);
    mcp2515.setFilter(MCP2515::RXF2, false, 0x7E8);
    mcp2515.setFilter(MCP2515::RXF3, false, 0x7E8);
    mcp2515.setFilter(MCP2515::RXF4, false, 0x7E8);
    mcp2515.setFilter(MCP2515::RXF5, false, 0x7E8);

    mcp2515.setNormalMode();

    Serial.println("CAN prêt (500kbps)");
}

// ─── Loop ────────────────────────────────────────────────────
void loop()
{
    delay(10);
}