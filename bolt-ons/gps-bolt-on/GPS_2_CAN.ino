#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// RX, TX pins connected to the GPS module
SoftwareSerial gpsSerial(40, 41);  // Adjust these pins based on your wiring
TinyGPSPlus gps;

void setup() {
  Serial.begin(9600);    // Initialize serial communication with the Serial Monitor
  gpsSerial.begin(9600); // Initialize serial communication with the GPS module

  Serial.println("GPS Module Initialized");
}

void loop() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    
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
