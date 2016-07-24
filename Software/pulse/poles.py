"""
poles.py

PULSE Main Controller
Pole LED Strips Interface

"""

import logging
logger = logging.getLogger(__name__)

import serial
from serial.serialutil import SerialException

import struct

class Poles:

    def __init__(self):
        logger.info("Initialising poles")
        
        if serial is None:
            self.ser = None
            logger.error("Serial library not initialised")
        else:
            try:
                self.ser = serial.Serial("COM4", 9600, timeout=0.5)
            except SerialException:
                logger.error("Error initialising serial link")
                self.ser = None

    def pulse(self, poles):
        self.update_cmd(0x00, poles[:3])

    def beacon(self, colours):
        poles = []
        for col in colours:
            poles.append([col, 0])
        self.update_cmd(0x01, poles)
       
    def hit(self, poles):
        self.update_cmd(0x04, poles)

    def set_update_speed(self, addr, period):
        period_bytes = list(struct.pack("!H",period))
        self.cmd(addr, 0x01, period_bytes)

    def update_cmd(self, cmdtype, poles):
        # poles: [[Colour1, Length1], ...]

        if self.ser is None:
            return

        #[Update][Type][LED1][SIZE1][LED2][SIZE2][LED3][SIZE3][CHECK]

        output = [0xFE, cmdtype]
        for pole in poles:
            output.extend(pole)
        output.append(0x00) # Fake Checksum
        bytesout = bytearray(output)

        try:
            self.ser.write(bytesout)
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

    def cmd(self, addr, cmd, data):

        if self.ser is None:
            return

        output = [0xFC, addr, cmd, len(data)] + data + [0xFD]
        bytesout = bytearray(output)

        try:
            self.ser.write(bytesout)
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None
