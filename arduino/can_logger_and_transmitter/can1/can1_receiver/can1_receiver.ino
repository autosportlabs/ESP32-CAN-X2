/*
 * ESP32 TWAI/CAN Receiver Example
 * 
 * Functionality:
 * - Initializes CAN controller at 500kbps
 * - Receives standard CAN frames (11-bit IDs)
 * - Prints all received messages to serial monitor
 * - Non-blocking message reception
 * 
 * Hardware Connections:
 * - RX: GPIO6, TX: GPIO7
 * - Requires external CAN transceiver
 * 
 * Usage:
 * - Connect to CAN bus with proper termination
 * - Monitor received messages at 115200 baud
 */

#include <driver/twai.h>

#define CAN1_RX_PIN GPIO_NUM_6
#define CAN1_TX_PIN GPIO_NUM_7


void CAN1_readMsg()
{
  twai_message_t message;

  while (twai_receive(&message, 0) == ESP_OK)  // Non-blocking
  {
    Serial.printf("[0x%X]: ", message.identifier);
    for (int i = 0; i < message.data_length_code; i++)
    {
      Serial.printf("0x%02X", message.data[i]);
      if (i < message.data_length_code - 1)
        Serial.print(", ");
    }
    Serial.println();
  }
}

bool setupCAN1(void)
{
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX_PIN, (gpio_num_t)CAN1_RX_PIN, TWAI_MODE_NORMAL);         // TWAI general configuration
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();            // TWAI timing configuration (500 kbps @ 80 MHz APB clock)
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();          // TWAI filter config - accept all

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)       // Install TWAI driver
  {
    Serial.println("Failed to install TWAI driver");
    return false;
  }

  // Start TWAI driver
  if (twai_start() != ESP_OK)
  {
    Serial.println("Failed to start TWAI driver");
    return false;
  }

  Serial.println("TWAI (CAN1) started. Waiting for messages...");
  return true;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing CAN1...");
  if (!setupCAN1())
  {
    Serial.println("Failed to setup CAN1. Exiting...");
    while (1);
  }
}

void loop()
{
  CAN1_readMsg(); // read CAN1 message
}
