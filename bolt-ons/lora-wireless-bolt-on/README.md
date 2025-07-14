# LoRa Communication with LoRa-Wireless-Bolt-On and ESP32-CAN-X2

## Overview
Repository includes demonstration codes LoRa communication using the LoRa-Wireless-Bolt-On LoRa module on a custom daughterboard with the ESP32-CAN-X2 development board. The setup utilizes the LoRa Arduino library by Sandeep Mistry (Version 0.8.0) for seamless communication between sender and receiver nodes.

## Requirements
- ESP32-CAN-X2 Development Board
- LoRa-Wireless-Bolt-On LoRa Module
- Arduino IDE
- LoRa Arduino Library
- Two ESP32 boards (one for sending, one for receiving)

## Installation
### Adding LoRa Library to Arduino IDE
1. Open Arduino IDE.
2. Navigate to **Sketch** → **Include Library** → **Add .ZIP Library...**.
3. Select the `LoRa` library ZIP file located in the `Library` folder of this repository.
4. Click "Open" to install the library.

Alternatively, you can manually copy the `LoRa` library folder to your Arduino libraries directory:
```
Documents/Arduino/libraries/
```

## Sample Codes
This repository includes two sample programs for testing LoRa communication:
1. `LoRa_Sender`: Sends a test message periodically.
2. `LoRa_Receiver`: Listens for incoming messages and prints them to the Serial Monitor.

### Uploading and Testing
#### LoRa Sender
1. Open `LoRa_Sender.ino` in Arduino IDE.
2. Select the correct ESP32 board and port under **Tools**.
3. Upload the code to your ESP32 sender board.
4. Open the Serial Monitor (115200 baud rate) to observe the transmission logs.

#### LoRa Receiver
1. Open `LoRa_Receiver.ino` in Arduino IDE.
2. Upload the code to another ESP32 board (receiver).
3. Open the Serial Monitor (115200 baud rate) to view received messages.

## Notes
- Ensure both sender and receiver use the same frequency settings.
- Antenna must be connected to the RA-02 module to avoid damage.
- LoRa works best in open areas with minimal obstructions.

## License
This project is open-source and free to use under the MIT License.

---
Developed for ESP32-CAN-X2 with LoRa-Wireless-Bolt-On LoRa Module.

