#ifndef BT_SERVICE_H
#define BT_SERVICE_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "19b10000-e8f2-537e-4f6c-d104768a1214"
#define OUT_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define IN_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"
#define CONTROL_CHARACTERISTIC_UUID "19b10003-e8f2-537e-4f6c-d104768a1214"

extern BLEServer* pServer;
extern BLECharacteristic* pOUTCharacteristic;
extern BLECharacteristic* pINCharacteristic;
extern BLECharacteristic* pControlCharacteristic;
extern bool deviceConnected;
extern bool oldDeviceConnected;
extern bool runningBLE;

void setupBLE();
void loopBLE();
void disableBLE();

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic);
};

#endif // BT_SERVICE_H
