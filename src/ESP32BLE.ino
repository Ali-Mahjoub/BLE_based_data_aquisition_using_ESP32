#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Adafruit_Sensor.h>
#include "DHT.h"

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

BLECharacteristic *characteristicTX; //through this object we will send data to the client
bool deviceConnected = false;//connected device control

#define LED 2

#define SERVICE_UUID           "ab0828b1-198e-4351-b779-901fa0e0371e" // UART service UUID
#define CHARACTERISTIC_UUID_RX "4ac8a682-9736-4e5d-932b-e9b31405049c"
#define CHARACTERISTIC_UUID_TX "0972EF8C-7613-4075-AD52-756F33D4DA91"

//callback to receive device connection events
class ServerCallbacks: public BLEServerCallbacks {
  
    void onConnect(BLEServer* pServer) {
 deviceConnected = true;
    };
    
    void onDisconnect(BLEServer* pServer) {
 deviceConnected = false;
    }
};


//callback for characteristic events

class CharacteristicCallbacks: public BLECharacteristicCallbacks {
     void onWrite(BLECharacteristic *characteristic) {
      
          //return pointer to register containing current value of characteristic
          std::string rxValue = characteristic->getValue();
           
          // check for data (size greater than zero)
          if (rxValue.length() > 0) {
               Serial.println("*********");
               Serial.print("Received Value: ");
               for (int i = 0; i < rxValue.length(); i++) {
               Serial.print(rxValue[i]);
               
               }
               Serial.println();
               
          }
     }
};
void setup() {
  Serial.begin(115200);
  //Serial.println(F("DHTxx test!"));
  pinMode(LED,OUTPUT);
  dht.begin();
  // Create the BLE Device
  BLEDevice::init("ESP32-BLE"); 
 
  // Create the BLE Server
  BLEServer *server = BLEDevice::createServer(); 
 
  server->setCallbacks(new ServerCallbacks()); 
 
  // Create the BLE Service
  BLEService *service = server->createService(SERVICE_UUID);
 
  // Create a BLE Characteristic for sending data
  characteristicTX = service->createCharacteristic(CHARACTERISTIC_UUID_TX,BLECharacteristic::PROPERTY_NOTIFY);
 
  characteristicTX->addDescriptor(new BLE2902());
  // Create a BLE Characteristic for receiving data
  BLECharacteristic *characteristic = service->createCharacteristic(CHARACTERISTIC_UUID_RX,BLECharacteristic::PROPERTY_WRITE);
 
  characteristic->setCallbacks(new CharacteristicCallbacks());
 
  // Start the service
  service->start();
 
  // Start advertising 
  server->getAdvertising()->start();
}

void loop() {
  //if there is any device connected
    if (deviceConnected) {
        // we call the sensor's "read" method to read the temperature
        //read will return 1 if it is able to read, or 0 otherwise
        float t = dht.readTemperature();
        if ( !isnan(t))
        {     if (t > 26.0) { 
               Serial.print("Turning LED ON!");
               digitalWrite(LED, HIGH);
        }
               else digitalWrite(LED, LOW);
               
            // retrieves the temperature reading of the object pointed out by the sensor
            // below we will convert the value to an array of char
            char txString[8]; 
            dtostrf(t, 2, 2, txString); // float_val, min_width, digits_after_decimal, char_buffer
            characteristicTX->setValue(txString); //set the value the feature will notify (send)
            characteristicTX->notify(); // Send the value to the smartphone
        }
    }
    delay(1000);
  // Wait 1 sec between measurements.
  delay(1000);
}
