#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pSensorCharacteristic = NULL;
BLECharacteristic* pLedCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
int bandera=0;
const int ledPin = 2; // Use the appropriate GPIO pin for your setup

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define LED_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"
String valor;



class MyServerCallbacks: public BLEServerCallbacks 
{
    void onConnect(BLEServer* pServer) 
    {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) 
    {
      deviceConnected = false;
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pLedCharacteristic) {
        std::string value = pLedCharacteristic->getValue();
        int len = value.length();

        if (len > 0) {
          Serial.println("*********");
          Serial.print("Valor: ");
          
          // Usar un buffer estático para concatenar eficientemente
          static char buffer[3000];
          snprintf(buffer, sizeof(buffer), "%s", value.c_str());

          // Imprimir el valor recibido
          Serial.print(buffer);

          // Acción basada en el valor recibido
          if (value == "enable_mqtt") {
            pSensorCharacteristic->setValue("{'enable_mqtt': 'true'}");
            pSensorCharacteristic->notify();
          }

          Serial.println();
          Serial.println("*********");
        }
    }
};

void setup() 
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Create the BLE Device
  BLEDevice::init("ESP323010");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pSensorCharacteristic = pService->createCharacteristic(
                      SENSOR_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create the ON button Characteristic
  pLedCharacteristic = pService->createCharacteristic(
                      LED_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  // Register the callback for the ON button characteristic
  pLedCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pSensorCharacteristic->addDescriptor(new BLE2902());
  pLedCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Esperando cliente...");
}

void loop() 
{

    // notify changed value
    if (deviceConnected && bandera==0) 
    {
        pSensorCharacteristic->setValue("{\"enable_mqtt\": true,\"mqtt_server\": \"inventoteca.com\",\"mqtt_port\": 1883,\"test\": true,\"id\": \"0\",\"enable_wifi\": true,\"ssid\": \"Inventoteca_2G\",\"pass\": \"science_7425\",\"email\": \"info@inventoteca.com\",\"ap\": \"GasSolutions\",\"ap_pass\": \"12345678\",\"firepass\": \"Homerosim\",\"key\": \"AIzaSyAOfGOjdpVQZtnhwTjmNRy8mApY993zHfc\"}");
        pSensorCharacteristic->notify();
        delay(3000);
        pSensorCharacteristic->setValue("{\"storage_id\": \"smart-industry-panels.appspot.com\",\"updated\": true,\"mainTime\": 10000,\"gmtOff\" : -21600,\"dayOff\" : 0,\"capacity\": 10000,\"percentage\": 70.5,\"pulsos_litro\": 179,\"folio\": 1,\"lat\": 19.03793, \"lon\": -98.20346,\"reporte\": 1,\"acumulado_litros\": 0}");
        pSensorCharacteristic->notify();
        delay(3000);
        bandera=1;
    }
    if (deviceConnected) 
    {
        // Crear el valor JSON y convertirlo a std::string
        String SValue = "{\"value\":" + String(value) + "}";
        std::string valueString = SValue.c_str();

        // Configurar el valor de la característica
        pSensorCharacteristic->setValue(valueString);
        pSensorCharacteristic->notify();
        value++;
        Serial.print("New value notified: ");
        Serial.println(value);
        delay(3000); // La pila Bluetooth se congestiona si se envían demasiados paquetes, en una prueba de 6 horas pude reducir el delay hasta 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) 
    {
        bandera=0;
        Serial.println("Device disconnected.");
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) 
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        Serial.println("Device Connected");
    }
}
