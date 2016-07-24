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
        logger.info("Initialising buttons")
        
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
        
    def check(self):
        if self.ser is None:
            return [False] * 5
        
        try:
            self.ser.write([0xC0])
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        try:
            rcvd = self.ser.read(size=1)
        except SerialException:
            logger.error("Error reading from serial port.")
            self.ser = None

        if len(rcvd) != 1 and (bytearray(rcvd)[0] & 0xe0 != 0xc0):
            logger.error("Error: buttons not returning proper data. Check connections and restart.")
            
            return [False] * 5
        
        rcvd = bytearray(rcvd)[0] & 0xFF
        
        buttons = [rcvd & 16 == 16, rcvd & 8 == 8, rcvd & 4 == 4, rcvd & 2 == 2, rcvd & 1 == 1]
        
        return buttons

    def update(self, colours):
        if self.ser is None:
            return

        colours = colours[:5]
        colours.extend([0] * (5 - len(colours)))
        output = bytearray([0xF0]+colours)

        try:
            self.ser.write(output)
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        try:
            rcvd = self.ser.read(size=2)
        except SerialException:
            logger.error("Error reading from serial port.")
            self.ser = None

        if rcvd != "OK":
            logger.error("Error: buttons not returning proper data. Check connections and restart.")
        
        return
