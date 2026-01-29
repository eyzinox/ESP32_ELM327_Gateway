// BLECom.cpp
#include "BLECom.hpp"
#define DEBUG_BLE_COM 1

void BLECom::begin()
{
    Serial.begin(115200);
    Serial.println("[BLE] Starting...");

    BLEDevice::init(DEVICE_NAME);
    BLEServer *server = BLEDevice::createServer();
    BLECallbacks *callbacks = new BLECallbacks(this);
    server->setCallbacks(callbacks);

    BLEService *service = server->createService(SERVICE_UUID);

    pTxCharacteristic = service->createCharacteristic(
        CHARACTERISTIC_TX,
        BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *rxCharacteristic = service->createCharacteristic(
        CHARACTERISTIC_RX,
        BLECharacteristic::PROPERTY_WRITE);
    rxCharacteristic->setCallbacks(callbacks);

    service->start();

    BLEAdvertising *adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->setScanResponse(false);
    adv->start();

    delay(500);
    Serial.println("[BLE] Ready!");
}

void BLECom::handleWrite(const char *data, size_t len)
{
#ifdef DEBUG_BLE_COM
    Serial.println("==== BLE RX ====");
    Serial.print("HEX : ");
    for (size_t i = 0; i < len; i++)
        Serial.printf("%02X ", (uint8_t)data[i]);
    Serial.println();

    Serial.print("ASCII : ");
    for (size_t i = 0; i < len; i++)
    {
        char c = data[i];
        if (c >= 32 && c <= 126)
            Serial.print(c);
        else
            Serial.print('.');
    }
    Serial.println("\n================");
#endif

    // ton buffer existant
    for (size_t i = 0; i < len; i++)
    {
        char c = data[i];
        if (c == '\n')
            continue;
        if (c == '\r')
        {
            if (rxIndex > 0 && elm)
            {
                rxBuffer[rxIndex] = 0; // termine la string
                size_t respLen = 0;
                const char *resp = elm->processCommand(rxBuffer, respLen);
                sendResponse(resp, respLen);
                rxIndex = 0;
            }
        }
        else
        {
            if (rxIndex < MAX_CMD_LEN - 1)
                rxBuffer[rxIndex++] = c;
            else
                rxIndex = 0; // overflow
        }
    }
}

void BLECom::sendResponse(const char *resp, size_t len)
{
#ifdef DEBUG_BLE_COM
    Serial.println("==== BLE TX ====");
    Serial.print("HEX : ");
    for (size_t i = 0; i < len; i++)
        Serial.printf("%02X ", (uint8_t)resp[i]);
    Serial.println();

    Serial.print("ASCII : ");
    for (size_t i = 0; i < len; i++)
    {
        char c = resp[i];
        if (c >= 32 && c <= 126)
            Serial.print(c);
        else
            Serial.print('.');
    }
    Serial.println("\n================");
#endif

    if (pTxCharacteristic)
    {
        pTxCharacteristic->setValue((uint8_t *)resp, len);
        pTxCharacteristic->notify();
    }
}
