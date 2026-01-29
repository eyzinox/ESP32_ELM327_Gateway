#ifndef BLECOM_HPP
#define BLECOM_HPP

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "ELMCom.hpp"

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
    ELMCom *elm = nullptr;

    class BLECallbacks : public BLEServerCallbacks, public BLECharacteristicCallbacks
    {
        BLECom *parent;

    public:
        BLECallbacks(BLECom *p) : parent(p) {}
        void onWrite(BLECharacteristic *c) override
        {
            String val = c->getValue();
            parent->handleWrite(val.c_str(), val.length());
        }
        void onDisconnect(BLEServer *s) override { s->getAdvertising()->start(); }
    };

public:
    void begin();
    void attachELM(ELMCom *e) { elm = e; }
    void handleWrite(const char *data, size_t len);
    void sendResponse(const char *resp, size_t len);
};

#endif // BLECOM_HPP