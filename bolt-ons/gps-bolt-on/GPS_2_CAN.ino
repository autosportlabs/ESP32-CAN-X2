#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include "mcp_can.h"

// RX, TX pins connected to the GPS module
SoftwareSerial gpsSerial(40, 41);  // Adjust these pins based on your wiring
TinyGPSPlus gps;

// CAN bus configuration
#define CAN0_INT 2  // Set interrupt pin for CAN module
MCP_CAN CAN(10);    // Set CS pin for CAN module

// CAN IDs for broadcasting
#define CAN_ID_LAT_LON 100
#define CAN_ID_SPEED_QUALITY 101

void setup() {
  Serial.begin(9600);    // Initialize serial communication with the Serial Monitor
  gpsSerial.begin(9600); // Initialize serial communication with the GPS module

  // Initialize CAN module
  if (CAN.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN BUS Shield Initialized Successfully!");
  } else {
    Serial.println("CAN BUS Shield Initialization Failed!");
    while (1);
  }
  
  Serial.println("GPS Module Initialized");
}

void loop() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c); // Parse the NMEA sentence

    if (gps.location.isUpdated()) {
      sendCANData();
    }
  }
}

void sendCANData() {
  // Latitude in signed decimal degrees
  int32_t lat = gps.location.lat() * 1000000; // Convert to 6 decimal places
  uint8_t latData[4];
  latData[0] = (lat >> 24) & 0xFF;
  latData[1] = (lat >> 16) & 0xFF;
  latData[2] = (lat >> 8) & 0xFF;
  latData[3] = lat & 0xFF;

  // Longitude in signed decimal degrees
  int32_t lon = gps.location.lng() * 1000000; // Convert to 6 decimal places
  uint8_t lonData[4];
  lonData[0] = (lon >> 24) & 0xFF;
  lonData[1] = (lon >> 16) & 0xFF;
  lonData[2] = (lon >> 8) & 0xFF;
  lonData[3] = lon & 0xFF;

  // Combine latitude and longitude data
  uint8_t latLonData[8];
  memcpy(latLonData, latData, 4);
  memcpy(latLonData + 4, lonData, 4);

  // Send latitude and longitude data on CAN ID 100
  CAN.sendMsgBuf(CAN_ID_LAT_LON, 0, 8, latLonData);

  // Speed in km/h
  uint16_t speed = gps.speed.kmph() * 100; // Convert to 2 decimal places
  uint8_t speedData[2];
  speedData[0] = (speed >> 8) & 0xFF;
  speedData[1] = speed & 0xFF;

  // GPS quality indicator based on the number of satellites
  uint8_t quality = gps.satellites.value();

  uint8_t speedQualityData[3];
  speedQualityData[0] = speedData[0];
  speedQualityData[1] = speedData[1];
  speedQualityData[2] = quality;

  // Send speed and GPS quality data on CAN ID 101
  CAN.sendMsgBuf(CAN_ID_SPEED_QUALITY, 0, 3, speedQualityData);

  Serial.println("Data Sent to CAN Bus");
}