import board
import digitalio
import busio
from time import sleep
from canio import CAN as CAN1
from adafruit_mcp2515 import MCP2515 as CAN2
from canio import Message, RemoteTransmissionRequest


# Setup CAN1 via canio
can1 = CAN1(rx=board.IO6, tx=board.IO7, baudrate=500_000, auto_restart=True)

# Setup CAN2 via adafruit_mcp2515
# CS pin for MCP2515 on CAN2
cs = digitalio.DigitalInOut(board.IO10)
cs.direction = digitalio.Direction.OUTPUT
spi = busio.SPI(board.IO12, board.IO11, board.IO13)
can2 = CAN2(spi, cs, baudrate=500_000)

while True:
    print("CAN1: Tx Errors:", can1.transmit_error_count,
    "Rx Errors:", can1.receive_error_count,
    "state:", can1.state)
    with can1.listen(timeout=1.0) as can1_listener:
        message_count = can1_listener.in_waiting()
        if message_count:
            print("CAN1: Messages available:", message_count)
            for _i in range(message_count):
                msg = can1_listener.receive()
                if isinstance(msg, Message):
                    print("CAN1: Recieved", msg.data, "from", hex(msg.id))
                if isinstance(msg, RemoteTransmissionRequest):
                    print("CAN1: RTR request length", msg.length, "from", hex(msg.id))

        send_success = can1.send(Message(id=0xf6, data=b"ping", extended=False))
        print("CAN1: Send success", send_success)

    with can2.listen(timeout=1.0) as can2_listener:
        message_count = can2_listener.in_waiting()
        if message_count:
            print("CAN2: Messages available:", message_count)
            for _i in range(message_count):
                msg = can2_listener.receive()
                if isinstance(msg, Message):
                    print("CAN2: Recieved", msg.data, "from", hex(msg.id))
                    send_sucess = can2.send(Message(id=0xf7, data=b"pong", extended=False))
                    print("CAN2: Send success", send_success)
                if isinstance(msg, RemoteTransmissionRequest):
                    print("CAN2: RTR request length", msg.length, "from", hex(msg.id))
    sleep(1)
