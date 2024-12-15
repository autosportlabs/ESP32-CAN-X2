#include <FastLED.h>
#include <driver/twai.h> // For TWAI (CAN)

#define LED_PIN        14       // GPIO pin connected to the LED strip
#define LED_BRIGHTNESS 30       // LED brightness (0 = off, 255 = maximum brightness)
#define LED_COUNT      10       // Number of LEDs in the strip

// Define CAN1 TX and RX pins
#define CAN1_TX 7  // Adjust based on your setup
#define CAN1_RX 6  // Adjust based on your setup
#define CAN_LED_ID 0x101        // CAN ID for individual LED control
#define CAN_ALERT_ID 0x102      // CAN ID for array (left/right) control

CRGB leds[LED_COUNT];           // Array to hold the LED data

// Function to initialize the CAN bus
void initCAN() {
    // General CAN configuration
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX, (gpio_num_t)CAN1_RX, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install CAN driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("TWAI driver installed successfully");
    } else {
        Serial.println("Failed to install TWAI driver");
    }

    // Start CAN driver
    if (twai_start() == ESP_OK) {
        Serial.println("TWAI driver started successfully");
    } else {
        Serial.println("Failed to start TWAI driver");
    }
}

void setup() {
    // Serial.begin(115200); // Initialize Serial monitor for debugging

    // Initialize the LED strip
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);
    FastLED.setBrightness(LED_BRIGHTNESS);
    fill_solid(leds, LED_COUNT, CRGB::Black); // Turn off all LEDs initially
    FastLED.show();

    // Initialize the CAN bus
    initCAN();
}

// Function to set the color of a specific LED
void setLEDColor(uint8_t index, byte red, byte green, byte blue) {
    if (index < LED_COUNT) { // Ensure the index is within bounds
        leds[index] = CRGB(red, green, blue);
        FastLED.show();
    }
}

// Function to set the color of an array (left or right) of LEDs
void setArrayColor(uint8_t index, byte red, byte green, byte blue) {
    uint8_t start = (index == 0) ? 0 : 5; // Determine start position based on index
    uint8_t end = start + 5;             // Determine end position
    if (end > LED_COUNT) end = LED_COUNT; // Ensure we don't go out of bounds

    for (uint8_t i = start; i < end; i++) {
        leds[i] = CRGB(red, green, blue);
    }
    FastLED.show();
}

void loop() {
    // Prepare a structure to hold incoming CAN messages
    twai_message_t rx_msg;

    // Check for incoming CAN messages
    if (twai_receive(&rx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {
        if (rx_msg.identifier == CAN_LED_ID && rx_msg.data_length_code == 4) {
            // Set a specific LED's color
            uint8_t ledIndex = rx_msg.data[0];
            byte red = rx_msg.data[1];
            byte green = rx_msg.data[2];
            byte blue = rx_msg.data[3];
            setLEDColor(ledIndex, red, green, blue);
        } 
        else if (rx_msg.identifier == CAN_ALERT_ID && rx_msg.data_length_code == 4) {
            // Set the color of an LED array (left or right)
            uint8_t arrayIndex = rx_msg.data[0];
            byte red = rx_msg.data[1];
            byte green = rx_msg.data[2];
            byte blue = rx_msg.data[3];
            setArrayColor(arrayIndex, red, green, blue);
        }
    }
}
