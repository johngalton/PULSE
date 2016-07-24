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
        logger.info("Song test")
        self.btns.update([1,2,3,5,6])
        logger.info("btn response")

        update = 20
        pole_delay = 181*update

     #   s = Song('C:\workspace\PULSE\Audio\Europe - The Final Countdown')
        s = Song('C:\workspace\PULSE\Audio\Coldplay - Viva the something')
     #   s = Song('C:\workspace\PULSE\Audio\Daft Punk - Derezzed')
     #   s = Song('C:\workspace\PULSE\Audio\Andy - Test Track')
     #   s = Song('C:\workspace\PULSE\Audio\Green Day - Boulevard of Broken Dreams')
     #   s = Song('C:\workspace\PULSE\Audio\Survivor - Eye of the tiger')
        s.set_update_speed(update)
        note_delay = s.delay + (20 * update)
        pole_delay = s.delay - pole_delay

        self.poles.set_update_speed(0x00, update)

        s.start()
        
        key_iter = s.notes.iterkeys()
        next_note = key_iter.next()
        
        poles = self.get_poles(s, next_note)
        
        note_keys = list(s.notes.keys())
        hit_index = 0
        hit_delay = 5 * update
        last_btn_update = 0
        
        while (s.isPlaying() == True):
            if next_note >= 0 and (s.time()-pole_delay) > (next_note):
                self.poles.pulse(poles)
                
                try:
                    next_note = key_iter.next()
                    poles = self.get_poles(s, next_note)
                
                except StopIteration:
                    next_note = -1;
                    poles = [[0,0],[0,0],[0,0]]
            
            if (hit_index < len(note_keys) and note_keys[hit_index] + note_delay - hit_delay < s.time()):
                if (note_keys[hit_index] + note_delay + hit_delay < s.time()):
                    hit_index += 1
                    print "miss"
                    continue
                
                state = self.btns.check()
                
                if True in state:
                    temp_hit_index = hit_index
                    
                    while (temp_hit_index < len(note_keys) and note_keys[temp_hit_index] + note_delay - hit_delay < s.time()):
                        match_key = True
                        
                        for n in s.notes[note_keys[temp_hit_index]]:
                            if state[n['pitch'] - 1] == False:
                                match_key = False
                                break
                        
                        if match_key == True:
                            print "hit!"
                            self.poles.hit(self.get_poles(s, note_keys[temp_hit_index]))
                            hit_index = temp_hit_index + 1
                            break
                        else:
                            temp_hit_index += 1

    def get_poles(self, s, next_note):
        poles = [[0,0],[0,0],[0,0]]
        
        for n in s.notes[next_note]:
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
        
        return poles
    
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
  #  pygame.init() doesn't work on my machine!
    pygame.mixer.init(frequency=22050, size=-16, channels=2, buffer=4096)

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
