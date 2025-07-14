/**
 * ESP32 LoRa Receiver with Status LEDs
 * 
 * Description:
 * Implements a LoRa packet receiver using the ESP32's HSPI interface.
 * - Initializes LoRa radio module with custom SPI pins
 * - Listens for incoming LoRa packets
 * - Displays received messages and signal strength via Serial
 * - Provides visual feedback using RX/TX LEDs
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
 *   - RX LED: GPIO8 (lights when packet received)
 *   - TX LED: GPIO9 (currently unused in this receive-only implementation)
 * 
 * Radio Settings:
 *   - Frequency: 433MHz
 *   - Uses HSPI interface
 * 
 * Functionality:
 * 1. Initializes Serial monitor (115200 baud)
 * 2. Sets up LED pins and LoRa module
 * 3. Continuously checks for incoming LoRa packets
 * 4. Prints received messages and RSSI to Serial
 * 5. Blinks RX LED on packet reception
 * 
 * Usage:
 * 1. Connect LoRa module to specified GPIO pins
 * 2. Upload sketch to ESP32
 * 3. Open Serial monitor (115200 baud)
 * 4. Monitor incoming LoRa packets and signal strength
 * 
 * Customization:
 * - Change frequency by modifying the 'frequency' constant
 * - Adjust LED pins by changing LED_RX/LED_TX defines
 * - Modify SPI pins if using different GPIOs
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

void setup()
{
  Serial.begin(115200); // initialize serial
  Serial.println("Serial Initialized");
  pinSetup();  // initialize pins
  LoRaSetup(); // initialize LoRa
}

void loop()
{
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    digitalWrite(LED_RX, LOW);
    Serial.print("Received packet '");
    while (LoRa.available())
    {
      Serial.print((char)LoRa.read());
    }

    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    digitalWrite(LED_RX, HIGH);
  }
}
