#include <driver/twai.h> // For TWAI (CAN)


// Define GPIO pins for buttons
#define BUTTON_1_PIN 21
#define BUTTON_2_PIN 47
#define BUTTON_3_PIN 48
#define BUTTON_4_PIN 38
#define CAN_TX_PIN GPIO_NUM_21 // CAN TX Pin
#define CAN_RX_PIN GPIO_NUM_20 // CAN RX Pin
#define CAN_BUTTON_STATE_ID 0x200 // CAN ID for periodic button state messages
#define CAN_BUTTON_EVENT_ID 0x201 // CAN ID for button event messages

// Button states
bool buttonStates[4] = {0, 0, 0, 0}; // Stores the current state of each button

// Initialize the TWAI configuration
void setupTWAI() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX, (gpio_num_t)CAN1_RX, TWAI_MODE_NORMAL);
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

// Read button states and return as a boolean array
void readButtonStates(bool states[]) {
    states[0] = !digitalRead(BUTTON_1_PIN); // Invert if buttons are active low
    states[1] = !digitalRead(BUTTON_2_PIN);
    states[2] = !digitalRead(BUTTON_3_PIN);
    states[3] = !digitalRead(BUTTON_4_PIN);
}

// Send periodic button states via CAN bus
void sendButtonStates(const bool states[]) {
    twai_message_t tx_msg;
    tx_msg.identifier = CAN_BUTTON_STATE_ID;
    tx_msg.extd = 0;
    tx_msg.data_length_code = 4;

    for (int i = 0; i < 4; i++) {
        tx_msg.data[i] = states[i];
    }

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {
        Serial.println("Button states sent via CAN bus.");
    } else {
        Serial.println("Failed to send button states.");
    }
}

// Send button state change event via CAN bus
void sendButtonEvent(uint8_t buttonId, bool state) {
    twai_message_t tx_msg;
    tx_msg.identifier = CAN_BUTTON_EVENT_ID;
    tx_msg.extd = 0;
    tx_msg.data_length_code = 2;

    tx_msg.data[0] = buttonId; // Button ID
    tx_msg.data[1] = state;   // Button state (1 = pressed, 0 = not pressed)

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {
        Serial.printf("Button %d state change sent: %d\n", buttonId + 1, state);
    } else {
        Serial.println("Failed to send button state change.");
    }
}

void setup() {
    //Uncomment the below to see debug output
    //Serial.begin(115200);

    // Configure button pins
    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);
    pinMode(BUTTON_3_PIN, INPUT_PULLUP);
    pinMode(BUTTON_4_PIN, INPUT_PULLUP);

    setupTWAI();
    Serial.println("Setup complete.");
}

void loop() {
    static bool lastButtonStates[4] = {0, 0, 0, 0}; // Previous button states
    static unsigned long lastPeriodicTime = 0;

    bool currentStates[4];
    readButtonStates(currentStates);

    // Send button state change events
    for (int i = 0; i < 4; i++) {
        if (currentStates[i] != lastButtonStates[i]) { // Detect state change
            sendButtonEvent(i, currentStates[i]);
            lastButtonStates[i] = currentStates[i]; // Update the last state
        }
    }

    // Send periodic button states every 100ms (10 Hz)
    if (millis() - lastPeriodicTime >= 100) {
        sendButtonStates(currentStates);
        lastPeriodicTime = millis();
    }

    delay(10); // Small delay to debounce and prevent busy loop
}

