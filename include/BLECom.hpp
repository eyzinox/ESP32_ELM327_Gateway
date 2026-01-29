// BLECom.hpp
#ifndef BLECOM_HPP
#define BLECOM_HPP

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "OBDCom.hpp"

#define DEVICE_NAME "ESP32_BLE_ELM327"
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_RX "12345678-1234-1234-1234-1234567890ac"
#define CHARACTERISTIC_TX "12345678-1234-1234-1234-1234567890ad"
#define MAX_CMD_LEN 64

class BLECom
{
private:
    BLECharacteristic *pTxCharacteristic = nullptr;
    char rxBuffer[MAX_CMD_LEN];
    uint8_t rxIndex = 0;
    OBDCom obd;

    void handleWrite(const char *data, size_t len);
    void sendResponse(const char *resp, size_t len);

    // Callback BLE unique
    class BLECallbacks : public BLEServerCallbacks, public BLECharacteristicCallbacks
    {
    private:
        BLECom *parent;

    public:
        BLECallbacks(BLECom *p) : parent(p) {}
        void onConnect(BLEServer *pServer) override
        {
            Serial.println("[BLE] Connected");
        }
        void onDisconnect(BLEServer *pServer) override
        {
            Serial.println("[BLE] Disconnected");
            pServer->getAdvertising()->start();
            // Serial.println("[BLE] Ready for new connection");
        }
        void onWrite(BLECharacteristic *pChar) override
        {
            String val = pChar->getValue();
            parent->handleWrite(val.c_str(), val.length());
        }
    };

public:
    BLECom()
    {
        rxIndex = 0;
        memset(rxBuffer, 0, sizeof(rxBuffer));
    }
    ~BLECom() {}

    void begin();
};

#endif
