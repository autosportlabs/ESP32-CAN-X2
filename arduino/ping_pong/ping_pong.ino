/*
  Simple example of sending/receiving messages between the two CAN interfaces on the ESP32-CAN-X2 dev board.
  https://wiki.autosportlabs.com/ESP32-CAN-X2
  
  This example relies on the ESP32 supplied TWAI api for interfacing with CAN1, and the Longan Labs mcp_canbus library
  for interfacing with the MCP2515 on CAN2.
*/

#include <Arduino.h>
#include <SPI.h>
#include "mcp_canbus.h"
#include "driver/twai.h"

#define POLLING_RATE_MS 1000

#define CAN1_ID  0xF6
#define CAN2_ID  0xF7

MCP_CAN CAN(CS);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Initializing builtin CAN peripheral");
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN1_TX, (gpio_num_t)CAN1_RX, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if(twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("CAN1 Driver initialized");
  } else {
    Serial.println("Failed to initialze CAN1 driver");
    return;
  }

  if (twai_start() == ESP_OK) {
    Serial.println("CAN1 interface started");
  } else {
    Serial.println("Failed to start CAN1");
    return;
  }

  uint32_t alerts_to_enable = TWAI_ALERT_TX_IDLE | TWAI_ALERT_TX_SUCCESS | TWAI_ALERT_TX_FAILED | TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    Serial.println("CAN1 Alerts reconfigured");
  } else {
    Serial.println("Failed to reconfigure alerts");
    return;
  }

  if (CAN_OK == CAN.begin(CAN_500KBPS)) {
    Serial.println("CAN2 interface started");
  } else {
    Serial.println("Failed to start CAN2");
    while (1);
  }
}

static void sendCAN1() {
  // Send message
  // Configure message to transmit
  twai_message_t message;
  message.identifier = CAN1_ID;
  message.extd = 0;
  message.rtr = 0;
  message.data_length_code = 4;
  message.data[0] = (int8_t)'p';
  message.data[1] = (int8_t)'i';
  message.data[2] = (int8_t)'n';
  message.data[3] = (int8_t)'g';

  // Queue message for transmission
  if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.println("CAN1: Message queued for transmission");
  } else {
    Serial.println("CAN1: Failed to queue message for transmission");
  }
}

static void readCAN1() {
  twai_message_t message;
  while (twai_receive(&message, 0) == ESP_OK) {

    Serial.print("CAN1: Received ");
    // Process received message
    if (message.extd) {
      Serial.print("extended ");
    } else {
      Serial.print("standard ");
    }

    if (message.rtr) {
      Serial.print("RTR ");
    }

    Serial.printf("packet with id 0x%x", message.identifier);

    if (message.rtr) {
      Serial.printf(" and requested length %d\n", message.data_length_code);
    } else {
      Serial.printf(" and length %d\n", message.data_length_code);
      Serial.printf("CAN1: Data: %.*s\n", message.data_length_code, message.data);
    }
  }
}

void sendCAN2() {
  unsigned char msg[4] = {'p', 'o', 'n', 'g'};

  if (CAN_OK == CAN.sendMsgBuf(CAN2_ID, 0, 4, msg)) {
    Serial.println("CAN2: Message queued for transmission");
  } else {
    Serial.println("CAN2: Failed to queue message for transmission");
  }
}

void readCAN2() {
  // try to parse packet
  unsigned char len = 0;
  unsigned char buf[8];

  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    Serial.print("CAN2: Received ");

    CAN.readMsgBuf(&len, buf);
    

    if (CAN.isExtendedFrame()) {
      Serial.print("extended ");
    } else {
      Serial.print("standard ");
    }

    if (CAN.isRemoteRequest()) {
      // Remote transmission request, packet contains no data
      Serial.print("RTR ");
    }

    Serial.printf("packet with id 0x%x", CAN.getCanId());

    if (CAN.isRemoteRequest()) {
      Serial.printf(" and requested length %d\n", len);
    } else {
      Serial.printf(" and length %d\n", len);
      Serial.printf("CAN2: Data: %.*s\n", len, buf);
      sendCAN2();
    }
    Serial.println();
  }
}


void loop() {
  Serial.println("MAIN: Disable LED");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);

  // Check if alert happened
  uint32_t alerts_triggered;
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));
  twai_status_info_t twaistatus;
  twai_get_status_info(&twaistatus);

  // Handle alerts
  if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
    Serial.println("CAN1: Alert: TWAI controller has become error passive.");
  }
  if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
    Serial.println("CAN1: Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");
    Serial.printf("CAN1: Bus error count: %d\n", twaistatus.bus_error_count);
  }
  if (alerts_triggered & TWAI_ALERT_TX_FAILED) {
    Serial.println("CAN1: Alert: The Transmission failed.");
    Serial.printf("CAN1: TX buffered: %d\t", twaistatus.msgs_to_tx);
    Serial.printf("CAN1: TX error: %d\t", twaistatus.tx_error_counter);
    Serial.printf("CAN1: TX failed: %d\n", twaistatus.tx_failed_count);
  }
  if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {
    Serial.println("CAN1: Alert: The RX queue is full causing a received frame to be lost.");
    Serial.printf("CAN1: RX buffered: %d\t", twaistatus.msgs_to_rx);
    Serial.printf("CAN1: RX missed: %d\t", twaistatus.rx_missed_count);
    Serial.printf("CAN1: RX overrun %d\n", twaistatus.rx_overrun_count);
  }
  if (alerts_triggered & TWAI_ALERT_TX_SUCCESS) {
    Serial.println("CAN1: Alert: The Transmission was successful.");
    Serial.printf("CAN1: TX buffered: %d\n", twaistatus.msgs_to_tx);
  }
  // Check if message is received
  if (alerts_triggered & TWAI_ALERT_RX_DATA) {
    readCAN1();
  }

  // Send message
  sendCAN1();


  Serial.println("MAIN: Enable LED");
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  readCAN2();
}
