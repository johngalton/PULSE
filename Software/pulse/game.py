"""
game.py

PULSE Main Controller
The actual game code

"""

import time
import logging
import math
import pygame

logger = logging.getLogger(__name__)


from .scoreboard import Scoreboard
from .buttons import Buttons
from .poles import Poles
from .song import Song



class Game:

    def __init__(self, songpath, btns, poles, scoreboard, scoreCallback):
        logger.info("Initialising game")

        self.btns = btns
        self.poles = poles
        self.scoreboard = scoreboard
        self.scoreCallback = scoreCallback

        self.update = 15
        self.pole_delay = 178 * self.update

        self.buttons_delay =  22 * self.update
        self.led_hit_window = 36
        self.hit_delay = (self.led_hit_window / 2) * self.update

        self.multiplierStreak = 0
        self.streak = 0
        self.score = 0
        self.high_streak = 0
        self.hit_count = 0
        self.miss_count = 0

        self.scoreboard.pulse()
        self.scoreboard.zeros(True)
        self.btns.update([1,2,3,5,6])

        startTone = pygame.mixer.Sound("C:\Users\Andrew\Documents\Github\PULSE\start_tone.wav")
        stopTone = pygame.mixer.Sound("C:\Users\Andrew\Documents\Github\PULSE\stop_tone.wav")

        self.scoreboard.set_text("3 3 3 3 ")
        startTone.play()
        time.sleep(0.7)
        self.scoreboard.set_text(" 2 2 2 2")
        startTone.play()
        time.sleep(0.7)
        self.scoreboard.set_text("1 1 1 1 ")
        startTone.play()
        time.sleep(0.7)
        stopTone.play()
        self.scoreboard.score(0)

        self.s = Song(songpath)

        self.s.set_update_speed(self.update)
        self.poles.set_update_speed(0x00, self.update)

    def play(self):
        self.s.start()

        note_delay = self.s.delay + self.buttons_delay
        pole_delay = self.s.delay - self.pole_delay

        notes = self.s.notes.copy()
        next_note = notes.popitem(last=False)
        next_poles = self.get_poles(next_note[1])
        notesLeft = True

        notes_btn = self.s.notes.copy()
        next_note_btn = notes_btn.popitem(last=False)
        next_poles_btn = self.get_poles(next_note_btn[1])

        windowStart = False
        buttonPressed = False
        buttonWasPressed = False
        buttonJustPressed = False
        noteHit = False
        buttonHoldFlag = False
        buttonHoldEnd = 0
        holdstateint = None

        notesLeftBtn = True
        while self.s.isPlaying():

            if notesLeft and self.s.time() >= next_note[0] + pole_delay:
                # Send next note to the poles
                self.poles.pulse(next_poles)
                self.poles.pulseTop(next_poles)

                try:
                    next_note = notes.popitem(last=False)
                    next_poles = self.get_poles(next_note[1])
                except KeyError:
                    # No more notes
                    notesLeft = False

            # Check Buttons
            btnstate = self.btns.check()
            btnstateint = self.btns_to_int(btnstate)
            notestateint = self.notes_to_int(next_note_btn[1])

            note_start = next_note_btn[0] + note_delay

            if True in btnstate:
                buttonPressed = True
            else:
                buttonPressed = False

            if buttonPressed and not buttonWasPressed:
                buttonJustPressed = True
            else:
                buttonJustPressed = False


            if buttonHoldFlag:
                if btnstateint == holdstateint and self.s.time() <= buttonHoldEnd :
                    self.updateHoldScore()
                else:
                    buttonHoldFlag = False

            # If in button window
            if not noteHit and notesLeftBtn and self.s.time() >= (note_start - self.hit_delay) and self.s.time() <= (note_start + self.hit_delay):
                windowStart = True

                if buttonJustPressed:

                    # Buttons match notes exactly
                    if btnstateint == notestateint:

                        if not noteHit:
                            # Register a hit
                            noteHit = True
                            self.poles.hit(next_poles_btn)
                            self.updateScore()

                            # Check if any are long notes
                            for note in next_note_btn[1]:
                                if note['len'] > 5:
                                    buttonHoldFlag = True
                                    buttonHoldEnd = note['dur']*1000 + note['time'] + note_delay + self.hit_delay
                                    holdstateint = btnstateint
                                    break

                    # Wrong Button/s
                    else:
                        self.multiplierStreak = 0

            # Not in a button window
            else:

                # Just finished a window
                if windowStart:
                    windowStart = False

                    # Missed the note
                    if not noteHit:
                        self.streak = 0
                        self.multiplierStreak = 0
                        self.miss_count += 1
                    noteHit = False

                    # Get next btn note
                    try:
                        next_note_btn = notes_btn.popitem(last=False)
                        next_poles_btn = self.get_poles(next_note_btn[1])
                    except KeyError:
                        # No more notes
                        notesLeftBtn = False

                if buttonPressed and not buttonHoldFlag:
                    self.multiplierStreak = 0

                if buttonJustPressed and not buttonHoldFlag:
                    self.streak = 0



            buttonWasPressed = buttonPressed

        #self.hiscore = yield self.endGameCallback(self.score)
        self.scoreboardShit()

        return self.score


    def scoreboardShit(self):
        self.scoreboard.pulse()
        self.btns.update([0,0,0,0,0])

        self.scoreboard.zeros(False)
        self.scoreboard.count_time(1000)

        #if self.hiscore:
        #    self.scoreboard.set_text("CONGRATS")
        #    time.sleep(1)
        #    self.scoreboard.set_text("HI-SCORE")
        #    time.sleep(1)
        #    self.scoreboard.set_text("        ")
        #    time.sleep(0.5)

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
        time.sleep(3)
        self.scoreboard.set_text(" OUT OF ")
        time.sleep(1)
        self.scoreboard.set_number(len(self.s.notes))
        time.sleep(2)

        self.scoreboard.set_text("  SCORE ")
        time.sleep(1)
        self.scoreboard.score(0)
        self.scoreboard.score(self.score)
        time.sleep(3)
        self.scoreboard.set_text("      ")

        self.scoreboard.pulse()

        self.scoreboard.zeros(True)
        self.scoreboard.count_time(300)


    def updateScore(self):
        #self.score += math.floor(10 * min(1 + (self.multiplierStreak / 5.0), 5))

        self.score += 10*(math.pow(2.0,(min(self.multiplierStreak/5.0, 4.0))))

        self.scoreboard.score(self.score)
        #yield self.scoreCallback(self.score)

        self.multiplierStreak += 1
        self.streak += 1
        self.high_streak = max(self.high_streak, self.streak)
        self.hit_count += 1

    def updateHoldScore(self):
        self.score += 2*(math.pow(2.0,(min((self.multiplierStreak/5.0), 4.0))))

        self.scoreboard.score(self.score)

    def btns_to_int(self, buttons):
        ret = 0
        for i in range(5):
            ret = (ret | (1 << i)) if buttons[i] else ret
        return ret

    def notes_to_int(self, notes):
        ret = 0

        for n in notes:
            ret = ret | (1 << (n['pitch'] - 1))

        return ret

    def get_poles(self, note):
        poles = [[0,0],[0,0],[0,0]]

        for n in note:
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
