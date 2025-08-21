/**
 * ESP32 LoRa Transmitter with Packet Counter
 * 
 * Description:
 * Implements a LoRa packet transmitter that sends incrementing counter messages.
 * Demonstrates basic LoRa communication with periodic message transmission.
 * 
 * Hardware Configuration:
 * LoRa Module Connections:
 *   - MOSI: GPIO15
 *   - MISO: GPIO16
 *   - SCK:  GPIO17
 *   - NSS:  GPIO18 (CS)
 *   - RST:  GPIO5
 *   - INT:  GPIO4 (DIO0)
 * 
 * LED Indicators:
 *   - RX LED: GPIO8 (unused in this transmit-only example)
 *   - TX LED: GPIO9 (unused in current implementation)
 * 
 * Radio Settings:
 *   - Frequency: 433MHz
 *   - Uses HSPI interface
 * 
 * Functionality:
 * 1. Initializes Serial monitor (115200 baud)
 * 2. Configures LoRa module with custom SPI pins
 * 3. Transmits "Hello World" messages with incrementing counter
 * 4. Sends packets every 1000ms
 * 
 * Usage:
 * 1. Connect LoRa module to specified GPIO pins
 * 2. Upload sketch to ESP32
 * 3. Open Serial monitor (115200 baud) for status messages
 * 4. Messages will be transmitted automatically every second
 * 
 * Customization:
 * - Change transmission interval by modifying the delay() value
 * - Adjust message content in LoRa_sendMessage()
 * - Modify frequency by changing the 'frequency' constant
 * - Enable TX LED blinking by adding digitalWrite() in LoRa_sendMessage()
 * 
 * Dependencies:
 * - LoRa library (Included in Repository)
 * - SPI library (built-in)
 */

#include <SPI.h> // include libraries
#include <LoRa.h>

#define LoRa_MOSI 15
#define LoRa_MISO 16
#define LoRa_RST 5
#define LoRa_NSS 18
#define LoRa_SCK 17
#define LoRa_INT 4

#define LED_RX 8
#define LED_TX 9

const long frequency = 433E6; // LoRa Frequency
SPIClass loRaSPI(HSPI);
void pinSetup()
{
  Serial.println("Starting pin Initialization");
  pinMode(LED_RX, OUTPUT);
  pinMode(LED_TX, OUTPUT);

  digitalWrite(LED_RX, HIGH);
  digitalWrite(LED_TX, HIGH);

  Serial.println("Pin Initialized");
}

void LoRaSetup()
{
  loRaSPI.begin(LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_NSS); // set LoRa SPI pins
  LoRa.setSPI(loRaSPI);                                    // set LoRa SPI
  LoRa.setPins(LoRa_NSS, LoRa_RST, LoRa_INT);              // set CS, reset, IRQ pin
  if (!LoRa.begin(frequency))
  {
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ;
  }

  Serial.println("LoRa init succeeded.");
  Serial.println();
}

void LoRa_sendMessage(String message)
{
  LoRa.beginPacket();   // start packet
  LoRa.print(message);  // add payload
  LoRa.endPacket(true); // finish packet and send it
}

void setup()
{
  Serial.begin(115200); // initialize serial
  Serial.println("Serial Initialized");
  pinSetup();  // initialize pins
  LoRaSetup(); // initialize LoRa
}

uint16_t counter = 0;

void loop() {
  LoRa_sendMessage("Hello World." + String(counter));
  counter++;
  delay(1000);
}
