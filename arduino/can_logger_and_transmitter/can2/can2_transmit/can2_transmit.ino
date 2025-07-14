/*
 * ESP32 MCP2515 CAN Bus Transmitter
 * 
 * Implements a CAN bus transmitter using MCP2515 controller with:
 * - Custom SPI pin configuration (HSPI bus)
 * - 500kbps baud rate (16MHz crystal)
 * - Support for standard CAN frames (11-bit IDs)
 * - Configurable message ID and payload
 * - Transmission status feedback
 * 
 * Hardware Configuration:
 * - SPI Interface:
 *   - SCK:  GPIO12
 *   - MISO: GPIO13  
 *   - MOSI: GPIO11
 *   - CS:   GPIO10
 * - Requires MCP2515 CAN controller module
 * - 120Ω termination resistor recommended on CAN bus
 * 
 * Default Transmission:
 * - Sends 8-byte test pattern every second
 * - Default CAN ID: 0x123
 * - Payload: 0x01 to 0x08
 * 
 * Dependencies:
 * - MCP_CAN library (https://github.com/coryjfowler/MCP_CAN_lib)
 */

 #include <mcp_can.h>            // ADD library https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

// Custom SPI Pins for MCP2515
#define CAN2_CS_PIN GPIO_NUM_10
#define CAN2_SPI_SCK GPIO_NUM_12
#define CAN2_SPI_MISO GPIO_NUM_13
#define CAN2_SPI_MOSI GPIO_NUM_11

SPIClass CAN2_SPI(HSPI); // Use default HSPI SPI bus
MCP_CAN CAN2(&CAN2_SPI, CAN2_CS_PIN);

bool setupCAN2()
{
  CAN2_SPI.begin(CAN2_SPI_SCK, CAN2_SPI_MISO, CAN2_SPI_MOSI, CAN2_CS_PIN);

  // Initialize MCP2515 with 500kbps and 16MHz crystal
  if (CAN2.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
  {
    CAN2.setMode(MCP_NORMAL);
    return true;
  }
  else
  {
    return false;
  }
}

void sendDataCAN2(byte *data, unsigned long data_len, unsigned long can_id)
{
  // CAN ID 0x123
  byte result = CAN2.sendMsgBuf(can_id, 0, data_len, data);

  if (result == CAN_OK)
  {
    Serial.println("Message sent successfully via CAN2");
  }
  else
  {
    Serial.print("Message send failed, code: ");
    Serial.println(result);
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  // Initialize SPI with custom pins
  if (!setupCAN2())
  {
    Serial.println("Failed to initialize CAN2. Check connections.");
    while (1)
      ;
  }
  else
  {
    Serial.println("MCP2515 Initialized Successfully (Sender)");
  }
  delay(1000);
}

void loop()
{
  // Create dummy data payload
  byte data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  unsigned long can_id = 0x123;                            // Example CAN ID
  unsigned long data_len = sizeof(data) / sizeof(data[0]); // Length of the data array

  sendDataCAN2(data, data_len, can_id);
  delay(1000);
}
