/*
  Project to combine GPS CAN bus broadcast with the GPS-bolt-on and also integrate a simple sensor reading using a GPIO. 

  System is powered by 12v DC, which is connected to the board with a 3 pin cable:
  * 12v
  * ground
  * sensor connection

  Sensor is connected to a digital switch that is powered by 12v. When the switch is activated, the switch output is connected to ground.
  It is assumed that when the switch output is not activated, there is a pullup to 12v. Therefore, the ESP32's GPIO input must be protected.
  
  Input circuit protection diagram:

      Sensor output ──[12 V pull-up]───┐
                                       │
                                      ┌┴┐
                                      │R│ 4.7K 1/4 W current limiting resistor
                                      └┬┘
                                       │
                                       +──► ESP32 GPIO 47
                                       │
                                      ┌┴┐
                                      --- cathode (band)
                                      | |
                                      │Z│ 1N4728A (3.3 V zener) to GND
                                      └┬┘
                                       │
                                      GND
  

  **GPS bolt-on Pin Configurations**
    - Rx to GPS module (tx from ESP32) - pin 17 of SV1 - GPIO40 of ESP32
    - Tx from GPS module (rx to ESP32) - pin 18 of SV1 - GPIO41 of ESP32
    - P1PPS from GPS module            - pin 16 of SV1 - GPIO39 of ESP32
    - PSE_SEL from GPS moduel          - pin 11 of SV1 - GPIO45 of ESP32

  **CAN Broadcasting the following information:**

  - **CAN ID 100**:
    - Latitude in signed decimal degrees. 4 byte payload (Multiply value by 1000000 to get 6 digits of precision, pack into 4 bytes)
    - CAN byte offset 0-3
    - Longitude in signed decimal degrees. 4 byte payload (Multiply value by 1000000 to get 6 digits of precision, pack into 4 bytes)
    - CAN byte offset 4-7

  - **CAN ID 101**:
    - Speed in km/h. 2 byte payload, offset 0 (Multiply by 100 to get 2 digits of precision, pack into two bytes)
    - GPS quality indicator. 1 byte payload, offset 2 (value is typically 0,1,2)
    - GPS Satellite count. 1 byte payload, offset 3
    - GPSDOP (dilution of precision). 2 byte payload, offset 4 (Multiply by 100 to get 2 digits of precision, pack into two bytes)

    ** CAN ID 102**
    - A counter to indicate the main program loop is basically alive (byte 0)
    - A sensor reading that sends 1 if the sensor output is activated (connected to ground) (byte 1)
*/

#include <Arduino.h>
#include <SPI.h>
#include "driver/twai.h"
#include <TinyGPS++.h>

// Initialize TinyGPS++
TinyGPSPlus gps;

// GPIO pin for level sensor
#define GPIO_SENSOR 47

// CAN1 IDs for broadcasting GPS data
#define CAN_ID_LAT_LNG 100
#define CAN_ID_SPEED_QUALITY 101
#define CAN_ID_COUNTER 102

// Define CAN1 TX and RX pins
#define CAN1_TX 7  // Adjust based on your setup
#define CAN1_RX 6  // Adjust based on your setup

// Desired update rate in Hz (1,5, 10, 25, 50)
static const uint8_t updateRateHz = 10;

// Baud-rate code for 115200 is 5
static const uint8_t baudCode115k = 5;

// A counter that will be broadcast to prove
// the system is alive
static uint8_t counter = 0;
static uint32_t last_counter_increment_time = 0;

// manages how often we send GPS broadcasts
// In your globals
static uint32_t last_gps_broadcast = 0;
const uint32_t gps_broadcast_interval = 1000 / updateRateHz;  // e.g. 100 ms

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

  Serial2.write(pkt, sizeof(pkt));
}

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

  Serial2.write(pkt, sizeof(pkt));
}

void setup() {
  Serial.begin(115200);    // Initialize serial communication with the Serial Monitor

  // Setup level sensor GPIO
  pinMode(GPIO_SENSOR, INPUT_PULLUP);

  // Setup GPS
  Serial2.begin(9600, SERIAL_8N1, 41, 40); // Initialize serial communication with the GPS module using hardware Serial2
  configureSkytraqBaudRate(baudCode115k);
  // give module a moment to switch
  delay(1000);
  Serial2.begin(115200);    
  configureSkytraqUpdateRate(updateRateHz);
  Serial.println("GPS Module Initialized");

  // CAN1 Initialization
  Serial.println("Initializing CAN1...");
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX, (gpio_num_t)CAN1_RX, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
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

void check_send_sensor() {
  if (millis() - last_counter_increment_time < 1000) {
    return;
  }

  counter++;
  last_counter_increment_time = millis();

  uint8_t is_activated = digitalRead(GPIO_SENSOR) == 0;

  twai_message_t message_counter;
  
  message_counter.identifier = CAN_ID_COUNTER;
  message_counter.extd = 0;                           // Standard Frame
  message_counter.rtr = 0;                            // Data Frame
  message_counter.data_length_code = 2;               // 1-byte payload
  message_counter.data[0] = counter;                  // set first byte to the counter
  message_counter.data[1] = is_activated;             // set the sensor value is activated

  if (twai_transmit(&message_counter, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.println("CAN1: tx counter/sensor");
  }
}

void broadcast_gps() {
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

  // Speed and GPS stats indicator
  int16_t speed = gps.speed.kmph() * 100;             // Convert speed to 2 decimal precision
  int16_t hdop_value = gps.hdop.value();              // Note, HDOP is pre-scaled to two digits of precision by tinyGPS

  uint8_t gps_quality;
  if (!gps.hdop.isValid()) {
    gps_quality = 0;                                  // no sat data at all
  }
  else if (gps.location.isValid()) {
    gps_quality = 2;                                  // full 3D fix
  }
  else {
    gps_quality = 1;                                  // just satellites / 2D fix
  }

  message_speed_quality.identifier = CAN_ID_SPEED_QUALITY;
  message_speed_quality.extd = 0;                           // Standard Frame
  message_speed_quality.rtr = 0;                            // Data Frame
  message_speed_quality.data_length_code = 6;               // 5-byte payload

  memcpy(&message_speed_quality.data[0], &speed, 2);        // First 2 bytes for speed
  message_speed_quality.data[2] = gps_quality;              // 1 byte for GPS quality indicator
  message_speed_quality.data[3] = gps.satellites.value();   //satellite count
  memcpy(&message_speed_quality.data[4], &hdop_value, 2);   // 2 bytes for HDOP

  // Send messages over CAN1
  if (twai_transmit(&message_lat_lng, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.println("CAN1: tx lat/lon");
  } else {
    Serial.println("CAN1: Failed to send Latitude and Longitude data");
  }

  if (twai_transmit(&message_speed_quality, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.println("CAN1: tx speed/qual");
  } else {
    Serial.println("CAN1: Failed to send Speed and GPS quality data");
  }
}

void check_send_gps() {
  while (Serial2.available() > 0) {
    //read in serial data, feed it to the GPS library
    //so it can construct GPS udpates    
    char c = Serial2.read();
    gps.encode(c); // Parse the NMEA sentence

    // time to broadcast?
    uint32_t now = millis();
    if (now - last_gps_broadcast >= gps_broadcast_interval) {
      broadcast_gps();
      last_gps_broadcast = now;
    }
  }
}

void loop() {
  check_send_sensor();
  check_send_gps();
}
