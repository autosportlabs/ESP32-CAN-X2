/*
  This code is for an ESP32-CAN-X2 microcontroller to read GPS data from a GPS module.
  - **TinyGPSPlus**: A library by Mikal Hart used to parse and decode NMEA sentences from GPS modules, providing easy access to location data such as latitude, longitude, speed, and quality indicators.
  The code initializes a hardware serial connection for the GPS module and parse GPS data.

  **Pin Configuration:**  
    - Rx to GPS module (tx from ESP32) - pin 17 of SV1 - GPIO40 of ESP32
    - Tx from GPS module (rx to ESP32) - pin 18 of SV1 - GPIO41 of ESP32
    - P1PPS from GPS module            - pin 16 of SV1 - GPIO39 of ESP32
    - PSE_SEL from GPS moduel          - pin 11 of SV1 - GPIO45 of ESP32
*/

#include <TinyGPS++.h>

// RX, TX pins connected to the GPS module
// Serial1 uses the hardware serial port and does not need pin assignment in code
TinyGPSPlus gps;

void setup() {
  Serial.begin(9600);    // Initialize serial communication with the Serial Monitor
  Serial1.begin(9600, SERIAL_8N1, 41, 40); // Initialize serial communication with the GPS module
  // Note: Replace 41 and 40 with actual RX and TX pins if required by your hardware.

  Serial.println("GPS Module Initialized");
}

void loop() {
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    
    gps.encode(c); // Parse the NMEA sentence

    if (gps.location.isUpdated()) {
      printGPSData();
    }
  }
}

void printGPSData() {
  // Latitude and Longitude
  Serial.print("Latitude: ");
  Serial.print(gps.location.lat(), 6);  // Six decimal places for precision
  Serial.print(" Longitude: ");
  Serial.println(gps.location.lng(), 6);

  // Number of Satellites
  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());

  // HDOP (Horizontal Dilution of Precision)
  Serial.print("HDOP: ");
  Serial.println(gps.hdop.hdop()); // Corrected the function call for HDOP

  // Speed in km/h
  Serial.print("Speed: ");
  Serial.print(gps.speed.kmph());  // Speed in kilometers per hour
  Serial.println(" km/h");

  Serial.println();
}
