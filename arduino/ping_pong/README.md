## IDE setup
### Selecting the board
In the Arduino IDE, choose the Autosport Labs ESP32-CAN-X2 board. For the port, choose the port that represents the USB port for the connected ESP32-CAN-X2

* Note: you may need to update your list of boards if you do not see it in the IDE

### Add support for mcp_canbus
In the library manager, install the mcp_canbus library by Longan Labs.

### Compiling the ping pong example
* Load the ping pong example into the IDE. 
* Press verify, and if there are no erros, upload to the board.
* After uploading, press the reset button on the ESP32-CAN-X2 board

You should see LED1 blinking at a regular interval

### Testing the ping pong example
* Refer to the wiring diagram on https://wiki.autosportlabs.com/ESP32-CAN-X2
* Connect the wires: 
  * CAN1 high to CAN2 high
  * CAN1 low to CAN2 low

Observe the messages in the serial monitor showing CAN bus activity

