# ESP32_and_BLE_:_Sensor_Data_Acquisation
![results_1](https://content.instructables.com/ORIG/FT0/V2LH/JM6KWMOL/FT0V2LHJM6KWMOL.png?auto=webp&frame=1&width=1024&fit=bounds&md=ef8b39b50ecfad88e42b75e8fe43c969)

<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#introduction">Introduction</a></li>  
    <li><a href="#the-ble-technology">The BLE technology</a></li>
      <ul>
        <li><a href="#introduction-to-ble">Introduction to BLE</a></li>
        <li><a href="#ble-structure">BLE Structure</a></li>
        <li><a href="#connection-procedure">Connection procedure</a></li>
      </ul>
    </li>       
    <li><a href="#code">Code</a></li>
    <ul>
        <li><a href="#implementation">Implementation</a></li>
        <li><a href="#test-and-results">Conclusion</a></li>
    </ul>
    <li><a href="#conclusion">Conclusion</a></li>
    <li><a href="#contact">Contact</a></li>
       
  </ol>
</details>

## Introduction
- One of the most beautiful features which the ESP32 has is the fact that, asides the WiFi, it has two other communication modules onboard. The ESP32 comes with an onboard Classic Bluetooth and Bluetooth Low Energy modules.So through this project we will focus on the BLE technology and we will execute a snippet of code to demonstrate how things work using a DHT11 sensor and ESP32 board.

## The BLE technology

### Introduction to BLE

- The Bluetooth Low Energy (BLE) was created to overcome the setbacks of classic Bluetooth which makes it a little bit unfit for use in IoT and battery powered smart devices which only need to send short burst of data at specific intervals. The BLE was designed to consume only a fraction of the power which classic Bluetooth devices consume when transmitting data and stay in sleep mode when not transmitting data unlike the Classic Bluetooth’s continuous data streaming. This makes BLE devices more power efficient and suitable for IoT products and other battery-powered smart devices which are usually desired to last for as long as possible on a single battery charge.
 
### BLE structure

- BLE uses a hierarchical data structure to send and receive information. A BLE device acting as a server will advertise services and characteristics that can be detected by a client and once the information exchange is successful, BLE devices can communicate with each other simultaneously. In technical terms, this information stack all together is known as an attribute of a BLE device. And it's defined and implemented using the GATT (Generic Attributes) profile. In these Profiles, we have Service, Characteristics, and values in a hierarchical order. Services contain characteristics and the characteristic contains the value, by reading the characteristic, we can read the values and values changes over time.


![results_1](https://infocenter.nordicsemi.com/topic/sds_s140/SDS/s1xx/Images/bt_stack_arch_s132_s140.svg)


Characteristics can be processed to include read or write information. Devices containing read components can publish information and devices which contain write characteristics can receive data from a client.

The GATT profile under which the services and characteristics are defined is known as a Universally Unique Identifier (UUID). There are some standard services and characteristics defined and reserved by the SIG corporation if we read the UUID of a BLE device, we can instantly tell what kind of device it is. More on this topic later.

BLE data is transmitted and received in very small packages, a BLE packet is of only 31 bytes in total when a TCP packet is of 60 bytes or more. Finally, one important thing to remember is that a BLE packet needs to be structured properly which can then be serialized and deserialized consistently in both the server and the client end. 

![results_1](https://doc.qt.io/archives/qt-5.5/images/peripheral-structure.png)

### Connection procedure

This schema represents multiple phases that the server and client devices go through to establish a connection and exchange data

![results_1](https://github.com/Ali-Mahjoub/BLE_based_data_aquisition_using_ESP32/blob/main/Images/BLE%20Connection%20procedure.PNG)

## Code

### Implementation

- This was a quite big annoying introduction to the BLE protocol (Yeah I know) so let's set a practical example to cover this up and understand things while programming.

- As i mentionned in the introduction, we will set the ESP32 board as a BLE device (SERVER) to send the DHT11 sensor data(Temperature) to an other BLE device (CLIENT).In this case
,i will use my phone as the client and connect it to the ESP32 and we will receive the data using a mobile app that supports BLE devices to interact with the server called BLE Scanner.


**Algorithm Structure**

  -The algorithm for the BLE server follows the explanation during the introduction above. We start by creating a BLE Service, after which we create BLE Characteristics under that service and a BLE descriptor under the characteristics. We then Start the service and start advertising so the device is visible to Scanning BLE devices.
  
  
-->We start the sketch by importing libraries that are required for the code.
```c
//BLE libraries
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
//DHT11 library
#include <Adafruit_Sensor.h>
#include "DHT.h"
```

-->Then we define and instanciate the sensor object and a led (ESP32 integrated led) 

```c
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
#define LED 2
```

-->Next, we provide the unique UUIDs for the Service and the Characteristics. These UUIDs can be generated using websites like UUID Generator

```c
#define SERVICE_UUID           "ab0828b1-198e-4351-b779-901fa0e0371e" 
#define CHARACTERISTIC_UUID_RX "4ac8a682-9736-4e5d-932b-e9b31405049c"
#define CHARACTERISTIC_UUID_TX "0972EF8C-7613-4075-AD52-756F33D4DA91"
BLECharacteristic *characteristicTX; //through this object we will send data to the client
bool deviceConnected = false;//connected device control
```
-->Then we create callbacks for the connection status events and for receiving data characteristic

* I recall here this the Callback function is called whenever the device receives some data from the user.
```c
//callback to receive device connection events

class ServerCallbacks: public BLEServerCallbacks {
  
    void onConnect(BLEServer* pServer) {
 deviceConnected = true;
    };
    
    void onDisconnect(BLEServer* pServer) {
 deviceConnected = false;
    }
};
```

* Here, we have the callback to receive device connection events.

```c
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
          }
     }
};
```

-->Next, We write the void setup() function. We start by initializing serial communication to be used for debugging purposes, after which we create an object of the BLEDevice class and set the object as a server.also we declare the led as output and initilise the sensor.

```c
void setup() {
  Serial.begin(115200);
  pinMode(LED,OUTPUT);
  
  //init sensor
  dht.begin();
  
  // Create the BLE Device
  BLEDevice::init("ESP32-BLE"); 
 
  // Create the BLE Server
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks()); 
```

-->And now we will create a service containing 2 characteristics for sending and receiving data.

```c
// Create the BLE Service
  BLEService *service = server->createService(SERVICE_UUID);
 
  // Create a BLE Characteristic for sending data
  characteristicTX = service->createCharacteristic(CHARACTERISTIC_UUID_TX,BLECharacteristic::PROPERTY_NOTIFY);
  characteristicTX->addDescriptor(new BLE2902());
  
  // Create a BLE Characteristic for receiving data
  BLECharacteristic *characteristic = service->createCharacteristic(CHARACTERISTIC_UUID_RX,BLECharacteristic::PROPERTY_WRITE);
  characteristic->setCallbacks(new CharacteristicCallbacks());
```

-->Finally, we start the service, setup parameters for advertising and start sending out advertising payload.
```c
// Start the service
  service->start();
 
  // Start advertising 
  server->getAdvertising()->start();
```

-->In the loop section,  we find that there is some device connected, and we try to read the sensor. If the action is carried out, we collect at room temperature. . We convert the value to a char array, set this value, and send it to the smartphone.if the temperature outruns certain predefined value the led is turned on.

```c
void loop() {
  //if there is any device connected
    if (deviceConnected) {
        // we call the sensor's "read" method to read the temperature
        
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
```
## Test and Results
![results_1](https://github.com/Ali-Mahjoub/azer/blob/main/Images/ESP_DHT11.jpg)
![results_1](https://github.com/Ali-Mahjoub/azer/blob/main/Images/Data%20received.jpg)

  ## Conclusion:
I impelemented through this snippet of code a simple ESP32 BLE server that sends data to my phone (BLE client) using a moblie app (BLE Scanner)and turns a warning led on when there is an overheat .
  ### Contact:
* Mail : ali.mahjoub1998@gmail.com 
* Linked-in profile: https://www.linkedin.com/in/ali-mahjoub-b83a86196/

