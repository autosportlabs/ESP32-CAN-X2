/*
toggle-buttons-can-broadcast demo

* toggles button states with LED status indicators
* broadcasts the current button state on CAN bus
*/

#include "driver/twai.h"
#include <FastLED.h>
#include <Arduino.h>

#define POLLING_RATE_MS 1000
#define BUTTON_CAN_ID 1000

#define LED_GPIO 14       // GPIO pin connected to the LED
#define LED_BRIGHT 100    // Brightness level (0-255)
#define LED_COUNT 10      // Total number of LEDs
#define BUTTON_COUNT 4

#define BUTTON_1 21
#define BUTTON_2 47
#define BUTTON_3 48
#define BUTTON_4 38


// Button state structure
struct ButtonState {
  uint32_t button_port;
  bool pressed;       // Current button state
  bool last_pressed;  // Previous button state
  bool activated;     // Current LED state (on/off)
};

// RGB structure
struct RGB {
  byte red;
  byte green;
  byte blue;
};

// Configure the RGB colors for active button states
RGB button_active_colors[] = {
  {0, 127, 127},
  {0, 127, 0},
  {127, 0, 127},
  {127, 127, 127}
};

// Inactive LED color
RGB inactive_color = {0, 0, 0};

// Button states
ButtonState button_states[] = {
  {BUTTON_1, false, true, false},
  {BUTTON_2, false, true, false},
  {BUTTON_3, false, true, false},
  {BUTTON_4, false, true, false},
};

// FastLED setup
CRGB leds[LED_COUNT]; // Array to hold LED colors

void setup() {
  // Uncomment for debug output
  // Serial.begin(115200);

  Serial.println("Initializing LEDs");
  FastLED.addLeds<WS2812, LED_GPIO, GRB>(leds, LED_COUNT); // Initialize LED strip
  FastLED.setBrightness(LED_BRIGHT);                      // Set brightness
  FastLED.clear();                                        // Clear LEDs (turn them off)
  FastLED.show();

  Serial.println("Initializing buttons");
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(button_states[i].button_port, INPUT_PULLUP); // Configure buttons as input with pull-up resistors
  }

  Serial.println("Initializing builtin CAN peripheral");
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX, (gpio_num_t)CAN1_RX, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("CAN1 driver initialized");
  } else {
    Serial.println("Failed to initialize CAN1 driver");
  }

  if (twai_start() == ESP_OK) {
    Serial.println("CAN1 interface started");
  } else {
    Serial.println("Failed to start CAN1");
    return;
  }
}

void setSwitchLEDs(int index, RGB color) {
  if (index < LED_COUNT - 1) { // Ensure index does not exceed LED array
    leds[index] = CRGB(color.red, color.green, color.blue);
    leds[index + 1] = CRGB(color.red, color.green, color.blue);
  }
}

void sendCANButtonStates() {
  twai_message_t message;
  message.identifier = BUTTON_CAN_ID;
  message.extd = 0;
  message.rtr = 0;
  message.data_length_code = 4;

  for (int i = 0; i < BUTTON_COUNT; i++) {
    message.data[i] = button_states[i].activated;
  }

  if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.println("CAN message queued for transmission");
  } else {
    Serial.println("Failed to queue CAN message");
  }
}

void loop() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    // Read the current button state
    button_states[i].pressed = digitalRead(button_states[i].button_port) == LOW;

    // Detect a button press (edge detection: HIGH to LOW)
    if (!button_states[i].last_pressed && button_states[i].pressed) {
      // Toggle the LED state
      button_states[i].activated = !button_states[i].activated;
    }

    // Update the last button state
    button_states[i].last_pressed = button_states[i].pressed;

    Set the LED color based on the toggled state
    if (button_states[i].activated) {
      setSwitchLEDs(i * 2, button_active_colors[i]); // Turn LEDs on
    } else {
      setSwitchLEDs(i * 2, inactive_color); // Turn LEDs off
    }
  }

  // Update LED strip
  FastLED.show();

  // Send button states over CAN
  sendCANButtonStates();

  // Debounce delay
  delay(50);
}
