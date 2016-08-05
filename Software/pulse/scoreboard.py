"""
scoreboard.py

PULSE Main Controller
Scoreboard interface

"""

import logging
import time

logger = logging.getLogger(__name__)

import serial
from serial.serialutil import SerialException

class Scoreboard:

    def __init__(self):
        logger.info("Initialising scoreboard")
        
        if serial is None:
            self.ser = None
            logger.error("Serial library not initialised")
        else:
            try:
                self.ser = serial.Serial("COM1", 9600, timeout=0.5)
            except SerialException:
                logger.error("Error initialising serial link")
                self.ser = None
    
    def score(self, score):
        if self.ser is None:
            return
        
        try:
            self.ser.write("score=" + str(score) + "\r")
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        return
    
    def set_number(self, score):
        if self.ser is None:
            return
        
        try:
            self.ser.write("num=" + str(score) + "\r")
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        return
    
    def set_text(self, text):
        if self.ser is None:
            return
        
        try:
            self.ser.write("text=" + text + "\r")
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        return

    def clear(self):
        if self.ser is None:
            return

        try:
            self.ser.write("clear\r")
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        return
    
    def pulse(self):
        if self.ser is None:
            return

        try:
            self.ser.write("clear\r")
            time.sleep(0.5)
            self.ser.write("pulse\r")
            time.sleep(2)
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        return
    
    def zeros(self, on):
        if self.ser is None:
            return

        try:
            if on == True:
                self.ser.write("showLeadingZeros=1\r")
            else:
                self.ser.write("showLeadingZeros=0\r")
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        return
    
    def count_time(self, time):
        if self.ser is None:
            return

        try:
            self.ser.write("countTime=" + str(time) + "\r")
        except SerialException:
            logger.error("Error writing to serial port.")
            self.ser = None

        return
