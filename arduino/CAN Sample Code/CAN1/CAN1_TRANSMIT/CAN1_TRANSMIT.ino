#include <driver/twai.h>

#define CAN1_TX_PIN GPIO_NUM_7
#define CAN1_RX_PIN GPIO_NUM_6

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

void sendCAN1Message()
{
  twai_message_t message;
  message.identifier = 0x123;     // Standard CAN1 ID
  message.extd = 0;               // Standard frame
  message.rtr = 0;                // Not a remote frame
  message.data_length_code = 8;  // 8-byte data frame

  for (int i = 0; i < 8; i++) {
    message.data[i] = i + 1;     // 0x01, 0x02, ..., 0x08
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
}

void loop()
{
  sendCAN1Message();
  delay(1000); // Send every 1 second
}
