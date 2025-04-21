#include <driver/twai.h>
#include <mcp_can.h>    // ADD library https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

#define CAN1_TX_PIN GPIO_NUM_7
#define CAN1_RX_PIN GPIO_NUM_6

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

  if (CAN2.begin(MCP_STDEXT, CAN_1000KBPS, MCP_16MHZ) == CAN_OK) {
    CAN2.setMode(MCP_NORMAL);
    return true;
  } else {
    return false;
  }
}


bool setupCAN1()
{
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX_PIN, (gpio_num_t)CAN1_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  // 500 kbps
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("Failed to install TWAI driver");
    return false;
  }

  if (twai_start() != ESP_OK) {
    Serial.println("Failed to start TWAI driver");
    return false;
  }

  Serial.println("TWAI (CAN1) sender started...");
  return true;
}

void sendCAN1Message(unsigned long canId, byte len, byte* data)
{
  twai_message_t message;
  message.identifier = canId;     // Standard CAN1 ID
  message.extd = 0;               // Standard frame
  message.rtr = 0;                // Not a remote frame
  message.data_length_code = len;  // 8-byte data frame

  for (int i = 0; i < message.data_length_code; i++) {
    message.data[i] = data[i]; 
  }

  esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(100));
  if (result == ESP_OK) {
    Serial.print("Message sent: [0x");
    Serial.print(message.identifier, HEX);
    Serial.print("] ");
    for (int i = 0; i < message.data_length_code; i++) {
      Serial.printf("0x%02X", message.data[i]);
      if (i < message.data_length_code - 1) Serial.print(", ");
    }
    Serial.println();
  } else if (result == ESP_ERR_TIMEOUT) {
    Serial.println("Transmit timeout – bus busy or buffer full");
  } else {
    Serial.printf("Transmit error: %s\n", esp_err_to_name(result));
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing CAN1 sender...");
  if (!setupCAN1()) {
    Serial.println("Setup failed. Halting.");
    while (1);
  }
  if (!setupCAN2()) {
    Serial.println("Failed to initialize CAN2. Check connections.");
    while (1);
  } else {
    Serial.println("MCP2515 Initialized Successfully (Receiver)");
  }
}

void receiveCAN2()
{
  if (CAN2.checkReceive() == CAN_MSGAVAIL) {
    long unsigned int canId;
    byte len;
    byte buf[8];

    CAN2.readMsgBuf(&canId, &len, buf);

    Serial.printf("Received Msg from [0x%03lX]: ", canId);
    for (byte i = 0; i < len; i++) {
      Serial.printf("0x%02X", buf[i]);
      if (i < len - 1) Serial.print(", ");
    }
    Serial.println();
  }
}
unsigned long prevTime = 0;
void loop()
{

  byte data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}; // Example data
  unsigned long canId = 0x123; // Example CAN ID  
  byte len = 6;
  unsigned long currentTime = millis();
  if (currentTime - prevTime >= 1000) { // Send every second
    sendCAN1Message(canId, len, data);  // Send CAN1 message
    prevTime = currentTime;
  }
  
  receiveCAN2(); // Check for incoming messages on CAN2
}