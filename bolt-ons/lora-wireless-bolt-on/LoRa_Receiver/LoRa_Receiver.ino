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
