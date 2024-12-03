#include "driver/twai.h"
#define POLLING_RATE_MS 1000

#include <Arduino.h>
#include <LiteLED.h>

#define BUTTON_CAN_ID  1000

#define LED_TYPE        LED_STRIP_WS2812
#define LED_GPIO 14     // GPIO pin connected to the LED
#define LED_BRIGHT 100  // Brightness level (0-255)
#define LED_COUNT 10    // Total number of LEDs
#define BUTTON_COUNT 4

#define BUTTON_1 21
#define BUTTON_2 47
#define BUTTON_3 48
#define BUTTON_4 38

struct RGB {
  byte red;
  byte green;
  byte blue;
};

struct ButtonState {
  uint32_t button_port;
  bool pressed;       // Current button state
  bool last_pressed;  // Previous button state
  bool led_state;     // Current LED state (on/off)
};

RGB pressed_color = {0, 127, 127};
RGB released_color = {0, 0, 0};

LiteLED myLED(LED_TYPE, 0); // Create the LiteLED object

ButtonState button_states[] = {
  {BUTTON_1, false, true, false},
  {BUTTON_2, false, true, false},
  {BUTTON_3, false, true, false},
  {BUTTON_4, false, true, false},
};

void setup() {
    // Uncomment to see serial debug output
    //Serial.begin(115200);
    Serial.println("Initializing LEDs");
    myLED.begin(LED_GPIO, LED_COUNT); // Initialize the LED object
    myLED.brightness(LED_BRIGHT);    // Set brightness
    myLED.fill(0x000000, 1);         // Set all LEDs to black (off)

    Serial.println("Initializing buttons");
    for (int i = 0; i < BUTTON_COUNT; i++) {
        pinMode(button_states[i].button_port, INPUT_PULLUP); // Configure buttons as input with pull-up resistors
    }



    Serial.println("Initializing builtin CAN peripheral");
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX, (gpio_num_t)CAN1_RX, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if(twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
      Serial.println("CAN1 Driver initialized");
    } else {
      Serial.println("Failed to initialze CAN1 driver");
    } 
    if (twai_start() == ESP_OK) {
      Serial.println("CAN1 interface started");
    } else {
      Serial.println("Failed to start CAN1");
      return;
    }
}

void setSwitchLEDs(int index, RGB color) {
    uint32_t pixel_color = (color.red << 16) | (color.green << 8) | color.blue;
    myLED.setPixel(index, pixel_color);
    myLED.setPixel(index + 1, pixel_color);
}


void sendCANButtonStates() {
    twai_message_t message;
    message.identifier = BUTTON_CAN_ID;
    message.extd = 0;
    message.rtr = 0;
    message.data_length_code = 4;
    for (int i = 0; i < BUTTON_COUNT; i++) {
      message.data[i] = button_states[i].led_state;
    }

    // Queue message for transmission
    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
      Serial.println("CAN1: Message queued for transmission");
    } else {
      Serial.println("CAN1: Failed to queue message for transmission");
    }  
}

void loop() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        // Read the current button state
        button_states[i].pressed = digitalRead(button_states[i].button_port) == LOW;

        // Detect a button press (edge detection: HIGH to LOW)
        if (!button_states[i].last_pressed && button_states[i].pressed) {
            // Toggle the LED state
            button_states[i].led_state = !button_states[i].led_state;
        }

        // Update the last button state
        button_states[i].last_pressed = button_states[i].pressed;

        // Set the LED color based on the toggled state
        if (button_states[i].led_state) {
            setSwitchLEDs(i * 2, pressed_color); // Turn LEDs on
        } else {
            setSwitchLEDs(i * 2, released_color); // Turn LEDs off
        }
    }
    sendCANButtonStates();

    myLED.show(); // Update the LEDs
    delay(50);    // Debounce delay
}
