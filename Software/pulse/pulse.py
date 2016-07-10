"""
pulse.py

PULSE Main Controller

"""

import sys
import logging
import signal
import time

from .buttons import Buttons
from .poles import Poles

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s | %(levelname)s | %(name)s: %(message)s")
logger = logging.getLogger("pulse.Pulse")

class Pulse:
    def __init__(self):
        logger.info("Initialising")

        self.btns = Buttons()
        self.poles = Poles()

    def main(self):
        while True:
            continue

    def btn_test(self):
        colour = [1,2,3,4,5,6]
        while(True):
            btnstate = self.btns.update(colour)
            if 1 in btnstate:
                colour = [colour[5]]+colour[:5]
            time.sleep(0.2)

    def pole_test(self):
        poles = [[0,0],[0,0],[0,0]]
        while True:
            for i in range(1,8):
                self.poles.beacon([i,0,0])
                time.sleep(0.5)
                poles[0] = [i,10]
                self.poles.pulse(poles)
                time.sleep(1)

    def pole_btn_test(self):
        colour = [1,2,3,5,6]
        poles = [[0,0],[0,0],[0,0]]
        beacons = [0,0,0]
        while(True):
            btnstate = self.btns.update(colour)
            print btnstate
            for i,state in enumerate(btnstate):
                if state == 1:
                    logger.info("Press")
                    self.poles.beacon([colour[i],0,0])
                    time.sleep(0.5)

                    poles[0] = [colour[i],5]
                    print poles
                    self.poles.pulse(poles)
                    time.sleep(1)


    def stop(self):
        logger.info("Quitting")
        sys.exit(0)

    def stop_signal(self, sig, frame):
        self.stop()

if __name__ == "__main__":
    pulse = Pulse()
    signal.signal(signal.SIGINT, pulse.stop_signal)

    args = str(sys.argv)
    if "btntest" in args:
        pulse.btn_test()
    elif "poletest" in args:
        pulse.pole_test()
    elif "pbtest" in args:
        pulse.pole_btn_test()
    else:
        pulse.main()
