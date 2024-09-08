/*
  This code is for an ESP32-CAN-X2 microcontroller to read GPS data from a GPS module and transmit it over a CAN bus network.
  - **TinyGPSPlus**: A library by Mikal Hart used to parse and decode NMEA sentences from GPS modules, providing easy access to location data such as latitude, longitude, speed, and quality indicators.
  The code initializes a hardware serial connection for the GPS module and a CAN bus interface for broadcasting the parsed GPS data.

  **Pin Configuration:**  
    - Rx to GPS module (tx from ESP32) - pin 17 of SV1 - GPIO40 of ESP32
    - Tx from GPS module (rx to ESP32) - pin 18 of SV1 - GPIO41 of ESP32
    - P1PPS from GPS module            - pin 16 of SV1 - GPIO39 of ESP32
    - PSE_SEL from GPS moduel          - pin 11 of SV1 - GPIO45 of ESP32

  **Broadcasting the following information:**

  - **CAN ID 100**:
    - Latitude in signed decimal degrees. 4-byte payload (Multiply value by 1000000 to get 6 digits of precision, pack into 4 bytes)
    - CAN byte offset 0-3
    - Longitude in signed decimal degrees. 4-byte payload (Multiply value by 1000000 to get 6 digits of precision, pack into 4 bytes)
    - CAN byte offset 4-7

  - **CAN ID 101**:
    - Speed in km/h. 2-byte payload (Multiply value by 100 to get 2 digits of precision, pack into two bytes)
    - CAN byte offset 0-1
    - GPS quality indicator. 1-byte payload (value is typically 0,1,2)
    - CAN byte offset 2
*/

#include <Arduino.h>
#include <SPI.h>
#include "driver/twai.h"
#include <TinyGPS++.h>

// Initialize TinyGPS++
TinyGPSPlus gps;

// CAN1 IDs for broadcasting GPS data
#define CAN_ID_LAT_LNG 100
#define CAN_ID_SPEED_QUALITY 101

// Define CAN1 TX and RX pins
#define CAN1_TX 7  // Adjust based on your setup
#define CAN1_RX 6  // Adjust based on your setup

void setup() {
  Serial.begin(9600);    // Initialize serial communication with the Serial Monitor
  Serial2.begin(9600, SERIAL_8N1, 41, 40); // Initialize serial communication with the GPS module using hardware Serial2

  Serial.println("GPS Module Initialized");

  // CAN1 Initialization
  Serial.println("Initializing CAN1...");
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX, (gpio_num_t)CAN1_RX, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("CAN1 Driver initialized");
  } else {
    Serial.println("Failed to initialize CAN1 driver");
    return;
  }

  if (twai_start() == ESP_OK) {
    Serial.println("CAN1 interface started");
  } else {
    Serial.println("Failed to start CAN1");
    return;
  }
}

void loop() {
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    gps.encode(c); // Parse the NMEA sentence

    if (gps.location.isUpdated()) {
      broadcastGPSData();
    }
  }
}

void broadcastGPSData() {
  // Prepare the messages
  twai_message_t message_lat_lng;
  twai_message_t message_speed_quality;

  // Latitude and Longitude
  int32_t latitude = gps.location.lat() * 1000000;    // Convert to signed decimal degrees with 6 digits of precision
  int32_t longitude = gps.location.lng() * 1000000;   // Convert to signed decimal degrees with 6 digits of precision

  message_lat_lng.identifier = CAN_ID_LAT_LNG;
  message_lat_lng.extd = 0;                           // Standard Frame
  message_lat_lng.rtr = 0;                            // Data Frame
  message_lat_lng.data_length_code = 8;               // 8-byte payload

  memcpy(&message_lat_lng.data[0], &latitude, 4);     // First 4 bytes for latitude
  memcpy(&message_lat_lng.data[4], &longitude, 4);    // Next 4 bytes for longitude

  // Speed and GPS quality indicator
  int16_t speed = gps.speed.kmph() * 100;             // Convert speed to 2 decimal precision
  uint8_t gps_quality = gps.hdop.isValid() ? 1 : 0;   // Use HDOP to determine GPS quality indicator (0,1,2)

  message_speed_quality.identifier = CAN_ID_SPEED_QUALITY;
  message_speed_quality.extd = 0;                     // Standard Frame
  message_speed_quality.rtr = 0;                      // Data Frame
  message_speed_quality.data_length_code = 3;         // 3-byte payload

  memcpy(&message_speed_quality.data[0], &speed, 2);  // First 2 bytes for speed
  message_speed_quality.data[2] = gps_quality;        // 1 byte for GPS quality indicator

  // Send messages over CAN1
  if (twai_transmit(&message_lat_lng, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.println("CAN1: Latitude and Longitude data sent");
  } else {
    Serial.println("CAN1: Failed to send Latitude and Longitude data");
  }

  if (twai_transmit(&message_speed_quality, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.println("CAN1: Speed and GPS quality data sent");
  } else {
    Serial.println("CAN1: Failed to send Speed and GPS quality data");
  }
}