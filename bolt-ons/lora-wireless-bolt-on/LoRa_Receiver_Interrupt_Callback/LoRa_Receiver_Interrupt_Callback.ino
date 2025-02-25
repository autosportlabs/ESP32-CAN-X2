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
bool rxFlag = false;
int packetSize = 0;
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

void onReceiveCB(int size)
{
  rxFlag = true;
  packetSize = size;
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

  LoRa.onReceive(onReceiveCB);
  LoRa.receive();
}

void readPacket()
{
  if (rxFlag == true)
  {
    digitalWrite(LED_RX, LOW);
    Serial.print("Received packet: ");
    for (int i = 0; i < packetSize; i++)
    {
      Serial.print((char)LoRa.read());
    }

    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    
    packetSize = 0;
    rxFlag = false;
    digitalWrite(LED_RX, HIGH);
  }
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
  readPacket();
}