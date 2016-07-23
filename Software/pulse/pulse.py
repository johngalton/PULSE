"""
pulse.py

PULSE Main Controller

"""

import sys
import logging
import signal
import time
import pygame

from .buttons import Buttons
from .poles import Poles
from .song import Song

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

    def song_test(self):
        self.btns.update([1,2,3,5,6])

        update = 15
        pole_delay = 190*update

        s = Song('/Users/daniel/Dropbox/PULSE Audio/Europe - The Final Countdown')
        s.set_update_speed(update)
        s.set_delay(pole_delay)

        self.poles.set_update_speed(0x00, update)

        s.start()

        for t,notes in s.notes.iteritems():
            poles = [[0,0],[0,0],[0,0]]
            for n in notes:
                pole = n['pitch']
                if pole <= 2:
                    poles[0][0] = pole
                    poles[0][1] = n['len']
                if pole == 3:
                    poles[1][0] = 3
                    poles[1][1] = n['len']
                if pole >= 4:
                    poles[2][0] = pole+1
                    poles[2][1] = n['len']


            while s.time()-s.delay < t*1000:
                continue

            self.poles.pulse(poles)


    def btn_test(self):
        colour = [1,2,3,4,5,6]
        while(True):
            btnstate = self.btns.update(colour)
            if 1 in btnstate:
                colour = [colour[5]]+colour[:5]
            time.sleep(0.25)

    def pole_test(self):
        poles = [[0,0],[0,0],[0,0]]
        self.poles.set_update_speed(0x00, 20)
        while True:
            for i in range(1,8):
                #self.poles.beacon([i,0,0])
                poles[0] = [1,200]
                poles[1] = [5,200]
                poles[2] = [2,200]
                self.poles.pulse(poles)
                return
                time.sleep(0.3)


    def pole_btn_test(self):
        colour = [1,2,3,5,6]
        poles = [[0,0],[0,0],[0,0]]
        beacons = [0,0,0]
        self.poles.set_update_speed(0x00, 12)
        while(True):
            btnstate = self.btns.update(colour)
            for i,state in enumerate(btnstate):
                if state == 1:
                    logger.info("Press")
                    self.poles.beacon([colour[i],0,0])
                    poles[0] = [colour[i],6]
                    poles[1] = [colour[i],6]
                    self.poles.pulse(poles)
                    #time.sleep(0.2)


    def stop(self):
        logger.info("Quitting")
        sys.exit(0)

    def stop_signal(self, sig, frame):
        self.stop()

if __name__ == "__main__":
    pulse = Pulse()
    signal.signal(signal.SIGINT, pulse.stop_signal)
    pygame.init()

    args = str(sys.argv)
    if "btntest" in args:
        pulse.btn_test()
    elif "poletest" in args:
        pulse.pole_test()
    elif "pbtest" in args:
        pulse.pole_btn_test()
    elif "songtest" in args:
        pulse.song_test()
    else:
        pulse.main()
