#include <mcp_can.h>    // ADD library https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

// Custom SPI Pins for MCP2515
#define CAN2_CS_PIN    GPIO_NUM_10
#define CAN2_SPI_SCK   GPIO_NUM_12
#define CAN2_SPI_MISO  GPIO_NUM_13
#define CAN2_SPI_MOSI  GPIO_NUM_11

SPIClass CAN2_SPI(HSPI);  // HSPI bus
MCP_CAN CAN2(&CAN2_SPI, CAN2_CS_PIN);

bool setupCAN2()
{
  CAN2_SPI.begin(CAN2_SPI_SCK, CAN2_SPI_MISO, CAN2_SPI_MOSI, CAN2_CS_PIN);

  if (CAN2.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    CAN2.setMode(MCP_NORMAL);
    return true;
  } else {
    return false;
  }
}

void CAN2_readMsg()
{
  if (CAN2.checkReceive() == CAN_MSGAVAIL) {
    long unsigned int canId;
    byte len;
    byte buf[8];

    CAN2.readMsgBuf(&canId, &len, buf);

    Serial.printf("[0x%03lX]: ", canId);
    for (byte i = 0; i < len; i++) {
      Serial.printf("0x%02X", buf[i]);
      if (i < len - 1) Serial.print(", ");
    }
    Serial.println();
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  if (!setupCAN2()) {
    Serial.println("Failed to initialize CAN2. Check connections.");
    while (1);
  } else {
    Serial.println("MCP2515 Initialized Successfully (Receiver)");
  }
}

/*************  ✨ Windsurf Command ⭐  *************/
/**
 * Continuously checks and reads available CAN messages.
 * This function is repeatedly called in the main loop to
 * process incoming CAN messages using the CAN2_readMsg function.
 */

/*******  271f6926-11a5-4a85-a008-3d620d11e8d8  *******/
void loop()
{
  CAN2_readMsg();  // Read CAN messages
}
