#include <LiteLED.h>
#include <driver/twai.h> // For TWAI (CAN)

#define LED_TYPE        LED_STRIP_WS2812

#define LED_GPIO 5      // GPIO pin connected to the LED strip
#define LED_BRIGHT 30   // LED brightness (0 = off, 255 = maximum brightness)
#define LED_COUNT 10    // Number of LEDs in the strip

#define CAN_TX_PIN GPIO_NUM_21 // CAN TX Pin (ESP32-S3)
#define CAN_RX_PIN GPIO_NUM_20 // CAN RX Pin (ESP32-S3)
#define CAN_LED_ID 0x101       // CAN ID for individual LED control
#define CAN_ALERT_ID 0x102     // CAN ID for array (left/right) control

LiteLED myLED(LED_TYPE, 0);    // Create the LiteLED object

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
    // Uncomment to see debug messages
    //Serial.begin(115200); // Initialize Serial monitor for debugging

    // Initialize the LED strip
    myLED.begin(LED_GPIO, LED_COUNT); 
    myLED.brightness(LED_BRIGHT); // Set LED brightness
    myLED.fill(0x000000, 1);      // Turn off all LEDs initially

    // Initialize the CAN bus
    initCAN();
}

// Function to set the color of a specific LED
void setLEDColor(uint8_t index, byte red, byte green, byte blue) {
    uint32_t color = (red << 16) | (green << 8) | blue; // Convert RGB to a 32-bit color
    myLED.setPixel(index, color); 
    myLED.show(); 
}

// Function to set the color of an array (left or right) of LEDs
void setArrayColor(uint8_t index, byte red, byte green, byte blue) {
    uint32_t color = (red << 16) | (green << 8) | blue; // Convert RGB to a 32-bit color
    uint8_t start = (index == 0) ? 0 : 5; // Determine start position based on index
    uint8_t end = start + 5;             // Determine end position
    for (uint8_t i = start; i < end; i++) {
        myLED.setPixel(i, color); 
    }
    myLED.show();
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
