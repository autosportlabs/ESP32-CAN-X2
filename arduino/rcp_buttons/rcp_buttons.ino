#include <Arduino.h>
#include "driver/twai.h"

#define HEARTBEAT_RATE_MS 50 // 20hz heartbeat
#define BUTTON_COUNT 4 // Number of configured buttons

#define CAN1_ID 0xF6

typedef struct {
  String name;
  uint8_t pin;
} Button;

// Define the GPIO -> Button mapping.  This uses pins 7-10 on SV1 on the ASL ESP-CAN-X2
Button buttons[BUTTON_COUNT] = {
  {"left", 14}, {"right", 21}, {"ok", 47}, {"no", 48}
};

uint8_t debounce_registers[BUTTON_COUNT]; // rolling buffers of bits for debounce, see https://deepbluembedded.com/arduino-button-debouncing/ for other options
uint8_t button_state; // bit packed button state
uint32_t last_can_ms; // Timestamp of the last can message in millis

void setup() {
  Serial.begin(115200);
  
  if (!setupCAN1()) {
    Serial.println("CAN1 initialization failure, aborting!");
    return;
  }

  if (!setupGPIOButtons()) {
    Serial.println("GPIO button intiialization failure, aborting!");
    return;
  }
}

bool setupGPIOButtons() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  return true;
}

bool setupCAN1() {
  Serial.println("Initializing builtin CAN peripheral");
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX, (gpio_num_t)CAN1_RX, TWAI_MODE_NORMAL);
  g_config.tx_queue_len = 1; // Only allow one message to be queued, ideally nothing ever sits in the queue
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if(twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("CAN1 Driver initialized");
  } else {
    Serial.println("Failed to initialze CAN1 driver");
    return false;
  }

  if (twai_start() == ESP_OK) {
    Serial.println("CAN1 interface started");
  } else {
    Serial.println("Failed to start CAN1");
    return false;
  }

  uint32_t alerts_to_enable = TWAI_ALERT_TX_IDLE | TWAI_ALERT_TX_SUCCESS | TWAI_ALERT_TX_FAILED | TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    Serial.println("CAN1 Alerts reconfigured");
  } else {
    Serial.println("Failed to reconfigure alerts");
    return false;
  }  

  return true;
}

void sendCANMessage(uint8_t button_state) {
  // Send message
  // Configure message to transmit
  twai_message_t message;
  message.identifier = CAN1_ID;
  message.extd = 0;
  message.rtr = 0;
  message.data_length_code = 1;
  message.data[0] = button_state;

  // Queue message for transmission
  esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(HEARTBEAT_RATE_MS));
  if (err == ESP_OK) {
    Serial.println("CAN1: Message queued for transmission");
  } else if (err == ESP_ERR_TIMEOUT) {
    Serial.println("CAN1: Transmission failed, timeout.");
  } else {
    Serial.println("CAN1: Unknown message transmit failure.");
  }
}

void loop() {
  uint8_t previous_state = button_state;
  uint32_t now = millis();
  uint32_t start_micros = micros();
  // Loop through the buttons, inverting the read (high == not pressed, low == pressed) and adding it to the
  // end of the debounce register, then update the button state if all bits are either 1 or 0 in the debounce
  // register
  for(int i = 0; i < BUTTON_COUNT; i++) {
    debounce_registers[i] = (debounce_registers[i] << 1) | !digitalRead(buttons[i].pin);
    if (debounce_registers[i] == 0xff) {
      button_state |= 1 << i;
    } else if (debounce_registers[i] == 0) {
      button_state &= 1 << i ^ 0xff;
    }
  }

  if (previous_state != button_state) {
    // The button state has changed, send a CAN message immediately
    sendCANMessage(button_state);
    last_can_ms = now;
  } else {
    // No state change, see if enough time has passed for another heartbeat
    if (last_can_ms == 0 || now - last_can_ms >= HEARTBEAT_RATE_MS) {
      sendCANMessage(button_state);
      last_can_ms = now;
    }
  }

  // Try to get the loop to run approx. every 1ms.
  uint32_t end_micros = micros();
  uint32_t micro_duration = 0;
  if (end_micros < start_micros) {
    micro_duration = (0xffffffff - start_micros) + end_micros;
  } else {
    micro_duration = end_micros - start_micros;
  }

  if (micro_duration < 1000) {
    delayMicroseconds(1000 - micro_duration);
  }
}
