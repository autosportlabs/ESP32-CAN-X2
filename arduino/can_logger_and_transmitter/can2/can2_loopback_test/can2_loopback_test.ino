/*
 * ESP32 MCP2515 CAN Bus Example (Loopback Mode)
 * 
 * Functionality:
 * - Initializes MCP2515 at 500kbps (16MHz crystal)
 * - Demonstrates loopback mode operation
 * - Sends test messages with ID 0x123
 * - Receives and prints looped-back messages
 * - Includes error checking for all operations
 * 
 * Hardware Connections:
 * - SPI: SCK=GPIO12, MISO=GPIO13, MOSI=GPIO11, CS: GPIO10
 * - Requires MCP2515 CAN controller module
 * 
 * Usage:
 * - Set to loopback mode for testing
 * - Monitor serial output at 115200 baud
 * - Change to normal mode for actual CAN bus communication
 */

#include <mcp_can.h>    // ADD library https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

// Custom SPI Pins
#define CAN2_CS_PIN       GPIO_NUM_10
#define CAN2_SPI_SCK      GPIO_NUM_12
#define CAN2_SPI_MISO     GPIO_NUM_13
#define CAN2_SPI_MOSI     GPIO_NUM_11

SPIClass CAN2_SPI(HSPI);  // Using HSPI
MCP_CAN CAN2(&CAN2_SPI, CAN2_CS_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(5000); // Give time for the Serial Monitor to open
  CAN2_SPI.begin(CAN2_SPI_SCK, CAN2_SPI_MISO, CAN2_SPI_MOSI, CAN2_CS_PIN);

  if (CAN2.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully (loopback mode)");
  } else {
    Serial.println("Error Initializing MCP2515");
    while (1);
  }

  CAN2.setMode(MCP_LOOPBACK);
  delay(1000);
}

void loop() {
  byte data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

  byte result = CAN2.sendMsgBuf(0x123, 0, 8, data);

  if (result == CAN_OK) {
    Serial.println("Message sent (loopback)");
  } else {
    Serial.print("Send failed, error code: ");
    Serial.println(result);
  }

  delay(100); 

  if (CAN2.checkReceive() == CAN_MSGAVAIL) {
    long unsigned int canId;
    byte len;
    byte recvData[8];
    CAN2.readMsgBuf(&canId, &len, recvData);

    Serial.print("[");
    Serial.print(canId, HEX);
    Serial.print("]: ");
    for (byte i = 0; i < len; i++) {
      Serial.print(recvData[i], HEX);
      if (i < len - 1) Serial.print(", ");
    }
    Serial.println();
  }

  delay(1000);
}
