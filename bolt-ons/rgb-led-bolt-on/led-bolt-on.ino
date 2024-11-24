#include <Adafruit_NeoPixel.h>
#include <driver/twai.h> // For TWAI (CAN)

#define LED_PIN 5          // WS2812B Data Pin
#define NUM_LEDS 10        // Total number of LEDs
#define CAN_TX_PIN GPIO_NUM_21 // CAN TX Pin (ESP32-S3)
#define CAN_RX_PIN GPIO_NUM_20 // CAN RX Pin (ESP32-S3)
#define CAN_LED_ID 0x101   // CAN ID for LED control
#define CAN_ALERT_ID 0x102 // CAN ID for Left/Right alerts

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Initialize the TWAI configuration
void setupTWAI() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("TWAI driver installed successfully.");
    } else {
        Serial.println("Failed to install TWAI driver.");
    }

    if (twai_start() == ESP_OK) {
        Serial.println("TWAI driver started successfully.");
    } else {
        Serial.println("Failed to start TWAI driver.");
    }
}

// Function to set a specific LED
void setPixelColor(uint8_t index, uint8_t red, uint8_t green, uint8_t blue) {
    if (index < NUM_LEDS) {
        strip.setPixelColor(index, strip.Color(red, green, blue));
        strip.show();
    }
}

// Function to set left or right alert
void setArrayColor(uint8_t arrayIndex, uint8_t red, uint8_t green, uint8_t blue) {
    uint8_t start = (arrayIndex == 0) ? 0 : 5; // 0 for left, 1 for right
    uint8_t end = start + 5;

    for (uint8_t i = start; i < end; i++) {
        strip.setPixelColor(i, strip.Color(red, green, blue));
    }
    strip.show();
}

void setup() {
    Serial.begin(115200);
    strip.begin();
    strip.show(); // Initialize all LEDs to off
    setupTWAI();
    Serial.println("Setup complete.");
}

void loop() {
    // Check for incoming CAN messages
    twai_message_t rx_msg;
    if (twai_receive(&rx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {
        if (rx_msg.identifier == CAN_LED_ID && rx_msg.data_length_code == 4) {
            // Set a specific LED
            uint8_t index = rx_msg.data[0];
            uint8_t red = rx_msg.data[1];
            uint8_t green = rx_msg.data[2];
            uint8_t blue = rx_msg.data[3];
            setPixelColor(index, red, green, blue);
        } else if (rx_msg.identifier == CAN_ALERT_ID && rx_msg.data_length_code == 4) {
            // Set left or right array
            uint8_t arrayIndex = rx_msg.data[0];
            uint8_t red = rx_msg.data[1];
            uint8_t green = rx_msg.data[2];
            uint8_t blue = rx_msg.data[3];
            setArrayColor(arrayIndex, red, green, blue);
        }
    }

    // Random color demo: Uncomment for testing
    
    // for (uint8_t i = 0; i < NUM_LEDS; i++) {
    //     setPixelColor(i, random(0, 256), random(0, 256), random(0, 256));
    //     setArrayColor(0, random(0, 256), random(0, 256), random(0, 256));    
    //     delay(1000);
    // }
    
}
