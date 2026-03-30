#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE
#define DEVICE_NAME "OBD_Logger"
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID_RX "12345678-1234-1234-1234-1234567890ac"
#define CHAR_UUID_TX "12345678-1234-1234-1234-1234567890ad"

BLECharacteristic *pTxCharacteristic = nullptr;

class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer) override
    {
        Serial.println("Client connecté");
    }

    void onDisconnect(BLEServer *pServer) override
    {
        Serial.println("Client déconnecté");
         BLEDevice::getAdvertising()->start();
    }
};

class RxCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pChar) override
    {
        if (!pChar)
            return;
        String val = pChar->getValue();
        Serial.printf("Reçu: %s\n", val.c_str());

        processCommand(val);
    }
};

void sendResponse(const char *resp)
{
    if (pTxCharacteristic)
    {
        Serial.printf("Réponse: %s\n", resp);
        pTxCharacteristic->setValue((uint8_t *)resp, strlen(resp));
        pTxCharacteristic->notify();
    }
}

void processCommand(String cmd)
{
    cmd.trim();
    cmd.toUpperCase();

    String response = "";

    if (cmd == "ATZ" || cmd == "ATI")
    { response = "ELM327 v1.5"; }
    else if (cmd == "ATDP")
    { response = "ISO 15765-4 (CAN 11/500)"; }
    else if (cmd == "ATDPN")
    { response = "A6"; }
    else if (cmd == "ATRV")
    { response = "13.8V"; }
    else if (cmd.startsWith("AT"))
    { response = "OK"; }
    else
    { response = "NO DATA"; }
    

    cmd = cmd.substring(0, 4); 
    if (cmd == "0100")
    { response = "7E8064100FFFFFFFF"; }
    else if (cmd == "010C")
    { response = "7E804410C0BB8"; }
    else if (cmd == "010D")
    { response = "7E803410D64"; }
    else if (cmd == "0105")
    { response = "7E80341005A0"; }

    if (cmd == "03")
    {
        // 1. Envoyer le First Frame
        sendResponse("7E8 10 09 43 03 00 3A 01 48\r\n");
        
        // 2. Petit délai pour laisser le temps à l'app de recevoir la notif BLE
        delay(20);
        
        // 3. Envoyer la Consecutive Frame
        sendResponse("7E8 21 00 FB 00 00 00 00 00\r\n>");
        return;
    }

    response += "\r\n>";
    sendResponse(response.c_str());
}

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
}

void loop()
{}