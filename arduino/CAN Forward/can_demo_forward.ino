/*
 * CAN Bus Bridge between ESP32 TWAI (CAN1) and MCP2515 (CAN2)
 *
 * Description:
 * This code creates a bidirectional bridge between two CAN interfaces:
 *   1. CAN1 - ESP32's native TWAI controller (CAN protocol)
 *   2. CAN2 - External MCP2515 module connected via SPI
 *
 * Functionality:
 * - Monitors CAN1 bus for incoming messages
 * - Forwards all received CAN1 messages to CAN2
 * - Prints all transactions to Serial monitor for debugging
 * - Handles basic error detection and reporting
 *
 * Hardware Configuration:
 * CAN1 (TWAI):
 *   - RX: GPIO6
 *   - TX: GPIO7
 *   - Speed: 250kbps
 * 
 * CAN2 (MCP2515):
 *   - SPI Bus: HSPI
 *   - SCK: GPIO12
 *   - MISO: GPIO13
 *   - MOSI: GPIO11
 *   - CS: GPIO10
 *   - Speed: 1Mbps
 *   - Crystal: 16MHz
 *
 * Libraries Required:
 * - driver/twai.h (ESP32 native)
 * - MCP_CAN_lib (https://github.com/coryjfowler/MCP_CAN_lib)
 * - SPI.h (Arduino/ESP32)
 *
 * Notes:
 * - Currently forwards messages unmodified
 * - Add message filtering/modification in CAN1_readMsg() if needed
 * - Serial monitor runs at 115200 baud
 */

#include <driver/twai.h>
#include <mcp_can.h>    // ADD library https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

// CAN1 (TWAI) Pins
#define CAN1_RX_PIN GPIO_NUM_6
#define CAN1_TX_PIN GPIO_NUM_7

// CAN2 (MCP2515) Custom SPI Pins
#define CAN2_CS_PIN GPIO_NUM_10
#define CAN2_SPI_SCK GPIO_NUM_12
#define CAN2_SPI_MISO GPIO_NUM_13
#define CAN2_SPI_MOSI GPIO_NUM_11

SPIClass CAN2_SPI(HSPI); // HSPI bus
MCP_CAN CAN2(&CAN2_SPI, CAN2_CS_PIN);

byte led_state = LOW;

bool sendDataCAN2(byte *data, unsigned long data_len, unsigned long can_id)
{

  Serial.printf("Sending message to CAN2 [0x%03lX]: ", can_id);
  for (byte i = 0; i < data_len; i++)
  {
    Serial.printf("0x%02X", data[i]);
    if (i < data_len - 1)
      Serial.print(", ");
  }
  Serial.println();

  // Send the message via CAN2
  byte result = CAN2.sendMsgBuf(can_id, 0, data_len, data);

  if (result == CAN_OK)
  {
    Serial.println("Message sent successfully via CAN2");
    return true;
  }
  else
  {
    Serial.printf("Message send failed, code: %d\n", result);
    return false;
  }
}

void CAN1_readMsg()
{
  twai_message_t message;

  while (twai_receive(&message, 0) == ESP_OK) // Non-blocking
  {
    // Print original CAN1 message
    Serial.printf("[CAN1 0x%X]: ", message.identifier);
    for (int i = 0; i < message.data_length_code; i++)
    {
      Serial.printf("0x%02X", message.data[i]);
      if (i < message.data_length_code - 1)
        Serial.print(", ");
    }
    Serial.println();

    // Here you can modify the data before it is sent to CAN2

    // Send modified message to CAN2
    if (sendDataCAN2(message.data, message.data_length_code, message.identifier)) {
        digitalWrite(LED_BUILTIN, led_state);
        led_state = led_state ? LOW : HIGH;
    }
  }
}

bool setupCAN1(void)
{
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX_PIN, (gpio_num_t)CAN1_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();   // CAN1 at 250 kbps
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL(); // Accept all messages

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
  {
    Serial.println("Failed to install TWAI driver");
    return false;
  }

  if (twai_start() != ESP_OK)
  {
    Serial.println("Failed to start TWAI driver");
    return false;
  }

  Serial.println("CAN1 (TWAI) started. Waiting for messages...");
  return true;
}

bool setupCAN2()
{
  CAN2_SPI.begin(CAN2_SPI_SCK, CAN2_SPI_MISO, CAN2_SPI_MOSI, CAN2_CS_PIN);

  if (CAN2.begin(MCP_STDEXT, CAN_1000KBPS, MCP_16MHZ) == CAN_OK)
  {
    CAN2.setMode(MCP_NORMAL);
    return true;
  }
  else
  {
    return false;
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  //configure LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup CAN1 (TWAI)
  Serial.println("Initializing CAN1...");
  if (!setupCAN1())
  {
    Serial.println("Failed to setup CAN1. Exiting...");
    while (1)
      ;
  }

  // Setup CAN2 (MCP2515)
  Serial.println("Initializing CAN2...");
  if (!setupCAN2())
  {
    Serial.println("Failed to initialize CAN2. Exiting...");
    while (1)
      ;
  }
}

void loop()
{
  CAN1_readMsg(); // Read from CAN1 and forward to CAN2 with data modification
}
