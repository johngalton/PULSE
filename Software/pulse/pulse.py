"""
pulse.py

PULSE Main Controller

"""

import os
import sys
import logging
import signal
import time
import pygame
import math

from autobahn import wamp
from autobahn.twisted.wamp import ApplicationSession, ApplicationRunner
from twisted.internet.defer import inlineCallbacks
from twisted.internet import reactor
from twisted.python.failure import Failure
from twisted.internet.defer import returnValue

from .scoreboard import Scoreboard
from .buttons import Buttons
from .poles import Poles
from .song import Song
from .game import Game

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s | %(levelname)s | %(name)s: %(message)s")
logger = logging.getLogger("pulse.Pulse")

songdirectory = "C:\workspace\PULSE\Audio\\"

class Pulse(ApplicationSession):
    def __init__(self, config):
        ApplicationSession.__init__(self, config)
        logger.info("Initialising")

        self.btns = Buttons()
        self.poles = Poles()
        self.scoreboard = Scoreboard()
        self.songs = []
        signal.signal(signal.SIGINT, self.stop_signal)

    @inlineCallbacks
    def onJoin(self, details):
        logger.info("Session Joined")

        subscribes = yield self.subscribe(self)
        for sub in subscribes:
            if isinstance(sub, Failure):
                logger.error("subscribe failed: {}".format(sub.getErrorMessage()))

        self.updateSongs()

        self.enabled = yield self.call(u'com.emfpulse.enabled.get')
        logger.info("Enabled: {}".format(self.enabled))
        if self.enabled:
            self.start()

    def onDisconnect(self):
        logger.error("Disconnected")

    def quit(self):
        logger.info("Quitting")
        reactor.stop()
        sys.exit(0)

    def stop_signal(self, sig, frame):
        self.quit()

    @inlineCallbacks
    def start(self):
        qn = yield self.checkForPlayerInQueue()
        logger.info(qn)
        if not qn:
            self.noplayer = True
            return

        self.scoreboard.pulse()
        self.scoreboard.set_text(str(qn)+" NEHT")
        self.btns.update([0,0,1,0,0])

        pretime = time.clock()
        while not self.btns.check()[2]: #While red button not pressed
            if time.clock() > pretime+30:
                self.publish(u'com.emfpulse.queue.toolate', qn)
                self.start()
                return

        info = yield self.call(u'com.emfpulse.queue.getnextinfo', qn)
        print info['artist']
        for song in self.songs:
            print song['artist']
            if song['name'] == info['song'] and song['artist'] == info['artist']:
                songpath = song['path']
                break

        res = yield self.call(u'com.emfpulse.playstatus.set', True)

        game = Game(songpath, self.btns, self.poles, self.scoreboard, self.publishScore)
        score = game.play()

        res = yield self.call(u'com.emfpulse.play.endgame', score)
        res = yield self.call(u'com.emfpulse.playstatus.set', False)

        self.enabled = yield self.call(u'com.emfpulse.enabled.get')
        if self.enabled:
            self.start()

    def publishScore(self, score):
        self.publish(u'com.emfpulse.current', {'score': int(score)})
        return

    @inlineCallbacks
    def checkForPlayerInQueue(self):

        q = yield self.call(u'com.emfpulse.queue.status')

        if q['queue_total'] > 0:
            qn = yield self.call(u'com.emfpulse.queue.next')
        else:
            qn = False

        returnValue(qn)

    @inlineCallbacks
    def updateSongs(self):
        for songpath in os.listdir(songdirectory):
            if os.path.isdir(songdirectory+songpath):
                logger.info(songpath)
                artist = songpath.split(" - ")[0]
                song = songpath.split(" - ")[1]
                self.songs.append({'artist': artist, 'name': song, 'path': songdirectory+songpath})

        yield self.call(u'com.emfpulse.songs.update', self.songs)

    @wamp.subscribe(u'com.emfpulse.enabled.status')
    def onEnabledStatus(self, i):
        wasenabled = self.enabled
        self.enabled = i
        logger.info("Enabled: {}".format(self.enabled))

        if self.enabled and not wasenabled:
            self.start()
        #elif not self.enabled and wasenabled:
        #    self.rain()

    @wamp.subscribe(u'com.emfpulse.queue')
    def onQueueChange(self, data):
        if data['queue_total'] > 0 and self.noplayer:
            self.noplayer = False
            if self.enabled:
                self.start()

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
                    poles[2] = [colour[i],6]
                    self.poles.pulse(poles)

if __name__ == "__main__":
    pygame.mixer.init(frequency=22050, size=-16, channels=2, buffer=4096)

    runner = ApplicationRunner(url=u"ws://emfpulse.com:12345/ws", realm=u'realm1')
    runner.run(Pulse)
