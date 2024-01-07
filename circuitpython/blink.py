import board
import digitalio
import time

# For the ESP32-CAN-X2 the LED is on IO2
led = digitalio.DigitalInOut(board.IO2)
led.direction = digitalio.Direction.OUTPUT

while True:
    led.value = True
    time.sleep(0.5)
    led.value = False
    time.sleep(0.5)
