#include <TinyGPSPlus.h>

/*
  Heavily based on the example BLE notify code provided with ESP32 on Arduino IDE. 
  Great video that goes over BLE concepts with ESP32: https://youtu.be/0Yvd_k0hbVs?si=qxlBrfBaO1deO5Bh 
  
  This basic example will create a BLE notify server and send updates everytime the GPS module updates.
  A 32 byte payload is created and transfered on each GPS update
    - bytes 0-7 latitude
    - bytes 8-15 longitude
    - bytes 16-23 speed m/s 
    - bytes 24-25 year
    - bytes 26 month
    - bytes 27 day 
    - bytes 28 hours
    - bytes 29 minute
    - bytes 30 second
    - bytes 31 centisecond

    **Pin Configuration:**  
    - Rx to GPS module (tx from ESP32) - pin 17 of SV1 - GPIO40 of ESP32
    - Tx from GPS module (rx to ESP32) - pin 18 of SV1 - GPIO41 of ESP32
    - P1PPS from GPS module            - pin 16 of SV1 - GPIO39 of ESP32
    - PSE_SEL from GPS moduel          - pin 11 of SV1 - GPIO45 of ESP32
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2901.h>
#include <TinyGPS++.h>

// Change to give device custom name that displays during GPS search
String deviceName = "ASL ESP32 + GPS";

// Change to whatever you'd like, good generator here: https://www.uuidgenerator.net 
// UUIDs are required 
#define SERVICE_UUID        "ECBB8159-FBA9-4123-AEF0-CA06E1D390D9"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// GPS Settings 

// Desired update rate in Hz (1,5, 10, 25, 50)
static const uint8_t updateRateHz = 10;
// Baud-rate code for 115200 is 5
static const uint8_t baudCode115k = 5;


BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLE2901 *descriptor_2901 = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// GPS parsing 
TinyGPSPlus gps; 

// type defs for packing gps data into bytes for ble
union latBytes {
  double lat;
  uint8_t bytes[8];
} latUnion; 

union longBytes {
  double lng;
  uint8_t bytes[8];
} longUnion; 

union speedBytes {
  double speed;
  uint8_t bytes[8];
} speedUnion; 

union yearBytes {
  int year;
  uint8_t bytes[2];
} yearUnion; 

union monthBytes {
  int month;
  uint8_t bytes[1];
} monthUnion; 

union dayBytes {
  int day;
  uint8_t bytes[1];
} dayUnion; 

union hourBytes {
  int hour;
  uint8_t bytes[1];
} hourUnion; 

union minBytes {
  int min;
  uint8_t bytes[1];
} minUnion; 

union secBytes {
  int sec;
  uint8_t bytes[1];
} secUnion; 

union centBytes {
  int cent;
  uint8_t bytes[1];
} centUnion; 

void configureSkytraqUpdateRate(uint8_t rateHz) {
  // Payload bytes: { MsgID=0x0E, Rate, Attributes=0 }
  const uint8_t payloadLen = 3;
  uint8_t payload[payloadLen] = { 0x0E, rateHz, 0x00 };

  // XOR‐checksum over the payload bytes
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < payloadLen; i++) {
    checksum ^= payload[i];
  }

  // Build the full packet:
  //  A0 A1 00 03 [payload…] [checksum] 0D 0A
  uint8_t pkt[2 + 2 + payloadLen + 1 + 2] = {
    0xA0, 0xA1,             // header
    0x00, payloadLen,       // payload length MSB=0, LSB=3
    payload[0], payload[1], payload[2], // payload
    checksum,               // XOR of payload[]
    0x0D, 0x0A              // terminator
  };

  Serial1.write(pkt, sizeof(pkt));
}


void configureSkytraqBaudRate(uint8_t baudCode) {
  // Payload bytes: { MsgID=0x05, COM_port=0, BaudCode, Attributes=0 }
  const uint8_t payloadLen = 4;
  uint8_t payload[payloadLen] = { 0x05, 0x00, baudCode, 0x00 };

  // XOR-checksum over the payload bytes
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < payloadLen; i++) {
    checksum ^= payload[i];
  }

  // Build the full packet:
  //  A0 A1 00 04 [payload…] [checksum] 0D 0A
  uint8_t pkt[2 + 2 + payloadLen + 1 + 2] = {
    0xA0, 0xA1,             // header
    0x00, payloadLen,       // payload length MSB=0, LSB=4
    payload[0], payload[1], payload[2], payload[3], // payload
    checksum,               // XOR of payload[]
    0x0D, 0x0A              // terminator
  };

  Serial1.write(pkt, sizeof(pkt));
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};


void setup() {
  Serial.begin(9600);

  // start reading from the gps module
  Serial1.begin(9600, SERIAL_8N1, 41, 40); // Initialize serial communication with the GPS module
  // Note: Replace 41 and 40 with actual RX and TX pins if required by your hardware.
  configureSkytraqBaudRate(baudCode115k);
  // give module a moment to switch
  delay(1000);
  Serial1.begin(115200);    
  configureSkytraqUpdateRate(updateRateHz);
  Serial.println("GPS Module Initialized");


  BLEDevice::init(deviceName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
  pCharacteristic->addDescriptor(new BLE2902());
  // Adds also the Characteristic User Description - 0x2901 descriptor
  descriptor_2901 = new BLE2901();
  descriptor_2901->setDescription("GPS Data");
  descriptor_2901->setAccessPermissions(ESP_GATT_PERM_READ);  // enforce read only - default is Read|Write
  pCharacteristic->addDescriptor(descriptor_2901);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

}


void loop() {
  while (Serial1.available()) {
      gps.encode(Serial1.read());
  }

  if (deviceConnected && gps.location.isUpdated()) {
    // parse GPS data
    latUnion.lat = gps.location.lat();
    longUnion.lng = gps.location.lng();
    speedUnion.speed = gps.speed.mps();
    yearUnion.year = gps.date.year();
    monthUnion.month = gps.date.month();
    dayUnion.day = gps.date.day();
    hourUnion.hour = gps.time.hour();
    minUnion.min = gps.time.minute();
    secUnion.sec = gps.time.second();
    centUnion.cent = gps.time.centisecond();

    // create data packet for BLE transfer
    uint8_t dataPacket[32];
    dataPacket[0] = latUnion.bytes[0];
    dataPacket[1] = latUnion.bytes[1];
    dataPacket[2] = latUnion.bytes[2];
    dataPacket[3] = latUnion.bytes[3];
    dataPacket[4] = latUnion.bytes[4];
    dataPacket[5] = latUnion.bytes[5];
    dataPacket[6] = latUnion.bytes[6];
    dataPacket[7] = latUnion.bytes[7];
    dataPacket[8] = longUnion.bytes[0];
    dataPacket[9] = longUnion.bytes[1];
    dataPacket[10] = longUnion.bytes[2];
    dataPacket[11] = longUnion.bytes[3];
    dataPacket[12] = longUnion.bytes[4];
    dataPacket[13] = longUnion.bytes[5];
    dataPacket[14] = longUnion.bytes[6];
    dataPacket[15] = longUnion.bytes[7];
    dataPacket[16] = speedUnion.bytes[0];
    dataPacket[17] = speedUnion.bytes[1];
    dataPacket[18] = speedUnion.bytes[2];
    dataPacket[19] = speedUnion.bytes[3];
    dataPacket[20] = speedUnion.bytes[4];
    dataPacket[21] = speedUnion.bytes[5];
    dataPacket[22] = speedUnion.bytes[6];
    dataPacket[23] = speedUnion.bytes[7];
    dataPacket[24] = yearUnion.bytes[0];
    dataPacket[25] = yearUnion.bytes[1];
    dataPacket[26] = monthUnion.bytes[0];
    dataPacket[27] = dayUnion.bytes[0];
    dataPacket[28] = hourUnion.bytes[0];
    dataPacket[29] = minUnion.bytes[0];
    dataPacket[30] = secUnion.bytes[0];
    dataPacket[31] = centUnion.bytes[0];
    
    // notify changed value
    if (deviceConnected) {
      pCharacteristic->setValue((uint8_t *)&dataPacket, 32);
      pCharacteristic->notify();
      delay(10);
    }
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}