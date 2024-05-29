## Getting CircuitPython
Download the .bin file from https://circuitpython.org/board/autosportlabs_esp32_can_x2/

## Flash circuitpython to the board

Follow these instructions for burning circuitpython .bin file using the GUI tool or esptool.py 
https://learn.adafruit.com/circuitpython-with-esp32-quick-start?view=all

* Note, this only needs to be done once - your python files will be uploaded to the board after this step is complete

## Connect ESP32-CAN-X2 to your computer
Connect the ESP32-CAN-X2 to your computer; after a moment, a removable drive should appear

## Copy your python code to the removable drive
Replace code.py with your own code. Ensure the filename remains code.py

## Example code
### blink.py
blink.py - blinks LED1

### ping_pong.py
ping_pong.py - sends CAN bus messages between CAN1 and CAN2.

In order to test, join these connections together:
* CAN1 High -> CAN2 High
* CAN1 Low -> CAN2 Low
