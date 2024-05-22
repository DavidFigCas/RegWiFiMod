#include "bt_service.h"
#include "system.h"

BLEServer* pServer = NULL;
BLECharacteristic* pOUTCharacteristic = NULL;
BLECharacteristic* pINCharacteristic = NULL;
BLECharacteristic* pControlCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool runningBLE = false;
uint32_t value = 0;

void MyServerCallbacks::onConnect(BLEServer* pServer) {
  deviceConnected = true;
}

void MyServerCallbacks::onDisconnect(BLEServer* pServer) {
  deviceConnected = false;
}

void MyCharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
  std::string value = pCharacteristic->getValue();

  if (value.length() > 0) {
    Serial.print("Valor recibido: ");
    Serial.println(value.c_str());

    // Deserializar el JSON recibido
    DynamicJsonDocument doc_patch(FILE_SIZE);
    DeserializationError error = deserializeJson(doc_patch, value.c_str());

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Combinar los objetos JSON
    for (const auto& kv : doc_patch.as<JsonObject>()) {
      obj[kv.key()] = kv.value();
    }

    //serializeJson(obj, Serial);
    //Serial.println();
    saveConfig = true;
  }
}


//---------------------------------------- setupBLE
void setupBLE() {
  if (runningBLE == false)
  {
    BLEDevice::init("ESP323010");
    BLEDevice::setMTU(517); // Tamaño máximo de MTU

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pOUTCharacteristic = pService->createCharacteristic(
                           OUT_CHARACTERISTIC_UUID,
                           BLECharacteristic::PROPERTY_READ   |
                           BLECharacteristic::PROPERTY_WRITE  |
                           BLECharacteristic::PROPERTY_NOTIFY |
                           BLECharacteristic::PROPERTY_INDICATE
                         );

    pINCharacteristic = pService->createCharacteristic(
                          IN_CHARACTERISTIC_UUID,
                          BLECharacteristic::PROPERTY_WRITE
                        );

    pControlCharacteristic = pService->createCharacteristic(
                               CONTROL_CHARACTERISTIC_UUID,
                               BLECharacteristic::PROPERTY_WRITE
                             );

    pOUTCharacteristic->addDescriptor(new BLE2902());
    pINCharacteristic->addDescriptor(new BLE2902());
    pControlCharacteristic->addDescriptor(new BLE2902());

    pOUTCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    pINCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    pControlCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    Serial.println("Esperando cliente...");
    runningBLE = true;
  }


}


//-----------------------------------------loopBLE
void loopBLE() 
{
  if (deviceConnected) 
  {
    String SValue;
    //String SValue = "{\"value\":" + String(value) + "}";


    serializeJson(status_doc, SValue);
    std::string valueString = SValue.c_str();

    //if (pOUTCharacteristic != nullptr) {
    pOUTCharacteristic->setValue(valueString);
    pOUTCharacteristic->notify();
    //Serial.println(valueString.c_str());
    //}
    //else
    // Serial.println("nullptr");
    value++;

    //Serial.println(value);
    //delay(3000);
    //value++;
    //Serial.print("New value notified: ");
    //Serial.println(value);
    //delay(3000);
  }
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");
    pServer->startAdvertising();
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected");
  }
}

void disableBLE() {
  Serial.println("Desactivando Bluetooth...");
  BLEDevice::deinit();
  runningBLE = false;
  Serial.println("Bluetooth desactivado");
}
