"""
poles.py

PULSE Main Controller
Pole LED Strips Interface

"""

import logging
logger = logging.getLogger(__name__)

import serial
from serial.serialutil import SerialException

class Poles:

    def __init__(self):

      if serial is None:
          self.ser = None
          logger.error("Serial library not initialised")
      else:
          try:
              self.ser = serial.Serial("/dev/tty.wchusbserial1420", 9600, timeout=0.5)
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
