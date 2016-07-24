"""
buttons.py

PULSE Main Controller
Physical Buttons Interface

5 Buttons
Transmit Packet, made up of 6x 8bit bytes
Colour bytes, preset values from lookup table, 0 to 255
[StartByte][Colour1][Colour2][Colour3][Colour4][Colour5]

Receive packet, made up of 6x 8bit bytes
Button bytes, only 0x00 or 0x01 representing not pressed and pressed
If no packet received back, there is a problem in button wiring
[StartByte][Button1][Button2][Button3][Button4][Button5]

"""

import logging
logger = logging.getLogger(__name__)

import serial
from serial.serialutil import SerialException

class Buttons:

    def __init__(self):

        if serial is None:
            self.ser = None
            logger.error("Serial library not initialised")
        else:
            try:
                self.ser = serial.Serial("COM3", 9600, timeout=0.5)
            except SerialException:
                logger.error("Error initialising serial link")
                self.ser = None

        self.state = [0] * 5
        self.rcvd = [0] * 6
        self.rcverr = 0

    def update(self, colours):

        if self.ser is None:
            return

        colours = colours[:5]
        colours.extend([0] * (5 - len(colours)))
        output = bytearray([127]+colours)

        try:
            self.ser.write(output)
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        try:
            rcvd = self.ser.read(size=6)
        except SerialException:
            logger.error("Error reading from serial port.")
            self.ser = None

        if len(rcvd) != 6:
            self.rcverr = self.rcverr + 1
        elif self.rcverr >= 3:
            logger.error("Error: buttons not returning proper data. Check connections and restart.")
            self.ser = None
        else:
            for i,byte in enumerate(rcvd[1:6]):
                self.state[i] = ord(byte)


        return self.state
