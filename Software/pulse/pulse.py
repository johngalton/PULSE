"""
pulse.py

PULSE Main Controller

"""

import sys
import logging
import signal
import time
import pygame
import math

from .scoreboard import Scoreboard
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
        self.scoreboard = Scoreboard()

    def main(self):
        while True:
            continue

    def song_test(self):
        self.scoreboard.pulse()
        self.scoreboard.zeros(True)
        self.scoreboard.score(0)
        
        self.btns.update([1,2,3,5,6])
    
        # Game configuration
        update = 15
        pole_delay = 180 * update
        buttons_delay = 3 * update
        led_hit_window = 30
        
        self.streak = 0
        self.score = 0
        self.high_streak = 0
        self.hit_count = 0
        self.miss_count = 0

     #   s = Song('C:\workspace\PULSE\Audio\Bastile - Bad Blood')
     #   s = Song('C:\workspace\PULSE\Audio\Gnarls Barkley - Crazy')
     #   s = Song('C:\workspace\PULSE\Audio\Junior Senior - Move Your Feet')
     #   s = Song('C:\workspace\PULSE\Audio\Sigala - Easy Love')
     #   s = Song('C:\workspace\PULSE\Audio\Alan Walker - Faded')
     #   s = Song('C:\workspace\PULSE\Audio\Europe - The Final Countdown')
     #   s = Song('C:\workspace\PULSE\Audio\Coldplay - Viva the something')
     #   s = Song('C:\workspace\PULSE\Audio\Daft Punk - Derezzed')
        s = Song('C:\workspace\PULSE\Audio\Andy - Test Track')
     #   s = Song('C:\workspace\PULSE\Audio\Green Day - Boulevard of Broken Dreams')
     #   s = Song('C:\workspace\PULSE\Audio\Survivor - Eye of the tiger')
        s.set_update_speed(update)
        note_delay = s.delay + buttons_delay
        pole_delay = s.delay - pole_delay

        self.poles.set_update_speed(0x00, update)
		
        startTone = pygame.mixer.Sound("C:\Users\Andrew\Documents\Github\PULSE\start_tone.wav")
        stopTone = pygame.mixer.Sound("C:\Users\Andrew\Documents\Github\PULSE\stop_tone.wav")

        s.start()
        
        key_iter = s.notes.iterkeys()
        next_note = key_iter.next()
        
        poles = self.get_poles(s, next_note)
        
        note_keys = list(s.notes.keys())
        hit_delay = (led_hit_window / 2) * update
        hit_index = 0
        last_btn_update = 0
        last_btn_state = 0
        last_note = None
        
        playedStart = 0
        
        #While the song is playing
        while (s.isPlaying() == True):
            #If the next notes time is not negative (end) and it is time to send the next note
            if next_note >= 0 and (s.time()-pole_delay) > (next_note):
                #Send the appropriate LED's
                self.poles.pulse(poles)
                
                #Try to find the next note
                try:
                    #Get the next note
                    next_note = key_iter.next()
                    #Get the colour and duration of the notes to send to the pole
                    poles = self.get_poles(s, next_note)
                #Catch exceeption
                except StopIteration:
                    #Cancel next note, send nothing to the poles.
                    next_note = -1;
                    poles = [[0,0],[0,0],[0,0]]
            
            #Get the button state
            state = self.btns.check()
            #Convert to integer (btn_state = int value of state)
            btn_state = self.btns_to_int(state)
            #Get the current song time
            cur_time = s.time()
            #Get the time of the current note, add note delay and see if we are out of notes set start to 0
            note_start = note_keys[hit_index] + note_delay if hit_index < len(note_keys) else 0
            
            #if we haven't run out of notes and we are after the start of the hit window
            if (hit_index < len(note_keys) and note_start - hit_delay < cur_time):
                #If we haven't already then play the start beep
                if (playedStart == 0):
                    startTone.play()
                    playedStart = 1;
                #If we're now past the end of the hit window
                #NOTE we will not land here if you hit the note
                if (note_start + hit_delay < cur_time):
                    #Resets streak
                    self.miss()
                    #Lets look for the next note now
                    hit_index += 1
                    #Play the stop tone, we never hit it... :(
                    stopTone.play()
                    #Reset the start tone flag
                    playedStart = 0;
                    #Forget the rest of the loop...
                    continue
                
                #If any button is pushed and the state has changed 
                if True in state and btn_state != last_btn_state: 
                    #Take some temporary values to keep track of current values
                    temp_hit_index = hit_index
                    temp_note_start = note_start
                    temp_time_key = note_keys[temp_hit_index]
                    
                    #While we are not at the end and we are after the current start of the hit window
                    while (temp_hit_index < len(note_keys) and temp_note_start - hit_delay < s.time()):
                        #Get what the notes should be
                        note_state = self.notes_to_int(s.notes[temp_time_key])
                        
                        #Check that the buttons are what they should be
                        if note_state == btn_state:
                            #If we've already hit a wrong note then kill the multiplier 
                            if hit_index != temp_hit_index:
                                self.miss()
                            
                            #Tell the poles you've hit and register a hit
                            self.poles.hit(self.get_poles(s, temp_time_key))
                            self.hit()
                            #increment hit index 
                            #hit_index = temp_hit_index + 1
                            hit_index = hit_index + 1 #JOHN - Otherwise if you've hit a wrong note first you will jump a load of notes

                            #If the note length > 2
                            if (s.notes[temp_time_key][0]['len'] > 2):
                                #Keep track of the notes time 
                                last_note = s.notes[temp_time_key]
                            #Break out of while loop 
                            break
                        #If wrong buttons were pressed
                        else:
                            #Increment our temp index
                            temp_hit_index += 1
                            
                            #If we haven't finished the song 
                            if temp_hit_index < len(note_keys):
                                #Keep track of the long notes start
                                temp_note_start = note_keys[temp_hit_index] + note_delay
                                #Keep track of the long notes key
                                temp_time_key = note_keys[temp_hit_index]
                            #Otherwise don't bother keeping track of the last buttons state
                            else:
                                break
            else:
                #If we've changed the buttons that are held
                if btn_state != last_btn_state:
                    #Clear the last note
                    last_note = None
                    
                    #If we've pressed something different
                    if self.btn_pressed(btn_state, last_btn_state):
                        #Count a miss
                        self.miss()
                #Otherwise if we're still holding a long note
                elif last_note != None and True in state:
                    #Calculate the end of this note
                    last_note_end = last_note[0]['dur'] + last_note[0]['time'] + note_delay
                    
                    #If we've reached the end of the note
                    if last_note_end < s.time():
                        #Clear the note
                        last_note = None
                    else:
                        #Otherwise add score
                        self.hold_score()
            
            #Track the last button state
            last_btn_state = btn_state
        
        self.scoreboard.pulse()
        
        self.scoreboard.zeros(False)
        self.scoreboard.count_time(1000)
        self.scoreboard.set_text(" LONGEST")
        time.sleep(1)
        self.scoreboard.set_text("   RUN  ")
        time.sleep(1)
        self.scoreboard.score(0)
        self.scoreboard.score(self.high_streak)
        time.sleep(3)
        
        self.scoreboard.set_text("   HIT  ")
        time.sleep(1)
        self.scoreboard.score(0)
        self.scoreboard.score(self.hit_count)
        time.sleep(2)
        self.scoreboard.set_text(" OUT OF ")
        time.sleep(1)
        self.scoreboard.score(0)
        self.scoreboard.score(len(note_keys))
        time.sleep(3)
        
        self.scoreboard.set_text("  SCORE ")
        time.sleep(1)
        self.scoreboard.score(0)
        self.scoreboard.score(self.score)
        time.sleep(3)
        
        self.scoreboard.pulse()
        
        self.scoreboard.zeros(True)
        self.scoreboard.score(self.score)
        self.scoreboard.count_time(300)
    
    def btn_pressed(self, btn_state, last_state):
        ret = False
        ret = True if btn_state & 1 != last_state & 1 and btn_state & 1 == 1 else False
        ret = True if btn_state & 2 != last_state & 2 and btn_state & 2 == 2 else False
        ret = True if btn_state & 4 != last_state & 4 and btn_state & 4 == 4 else False
        ret = True if btn_state & 8 != last_state & 8 and btn_state & 8 == 8 else False
        ret = True if btn_state & 16 != last_state & 16 and btn_state & 16 == 16 else False
        
        return ret
    
    def btns_to_int(self, buttons):
        ret = 0
        ret = (ret | (1 << 0)) if buttons[0] == True else ret
        ret = (ret | (1 << 1)) if buttons[1] == True else ret
        ret = (ret | (1 << 2)) if buttons[2] == True else ret
        ret = (ret | (1 << 3)) if buttons[3] == True else ret
        ret = (ret | (1 << 4)) if buttons[4] == True else ret
        
        return ret
    
    def notes_to_int(self, notes):
        ret = 0
        
        for n in notes:
            ret = ret | (1 << (n['pitch'] - 1))
        
        return ret
    
    def hit(self):
        # print "score"
        self.score += math.floor(10 * min(1 + (self.streak / 5.0), 5))
        self.scoreboard.score(self.score)
        
        self.streak += 1
        self.high_streak = max(self.high_streak, self.streak)
        self.hit_count += 1
    
    def hold_score(self):
        # print "hold score"
        self.score += min(1 + (self.streak / 5), 5)
        self.scoreboard.score(self.score)
    
    def miss(self):
        # print "miss"
        self.streak = 0;
        self.miss_count += 1
    
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
