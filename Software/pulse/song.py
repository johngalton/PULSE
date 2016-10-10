"""
poles.py

PULSE Main Controller
Song Parser

name, artist, delay, notes[time:{pitch, dur}]

"""
import os
import ConfigParser
import logging
import midi
import collections
import threading
import pygame

logger = logging.getLogger(__name__)
import time

class Song:
    def __init__(self, location):
        self.location = location
        if not os.path.isdir(location):
            logger.error("Song directory does not exist")
            return
        if not os.path.exists(location+'/song.ini') or not os.path.exists(location+'/guitar.ogg') or not os.path.exists(location+'/notes.mid'):
            logger.error("One or more files missing from directory")
            return


        self.read_ini()
        self.read_midi()
        
        pygame.mixer.music.load(self.location+'/guitar.ogg')

    def start(self):
        pygame.mixer.music.play()

    def time(self):
        return pygame.mixer.music.get_pos()

    def set_update_speed(self, rate):
        # Rate in ms
        self.update_rate = rate
        for t,items in self.notes.iteritems():
            for note in items:
                x = int(round(note['dur']/(rate/1000.0)))
                #x = 2 if x < 2 else x       #increased
                #x = x - 4 if x > 10 else x   #added
                x = max(x-6, 2)
                note['len'] = x

    def set_delay(self, time):
        self.delay -= time

    def read_ini(self):
        config = ConfigParser.ConfigParser()
        config.read(self.location + '/song.ini')
        self.artist = config.get('song', 'artist')
        self.name = config.get('song', 'name')
        self.delay = config.getfloat('song', 'delay') # Seconds
        self.top_note = config.getint('song', 'top_note')

    def read_midi(self):

        pattern = midi.read_midifile(self.location+'/notes.mid')
        self.resolution = float(pattern.resolution)
        pattern.make_ticks_abs()

        for track in pattern:
            if len([True for x in track if isinstance(x, midi.SetTempoEvent)]) > 0:
                tempo_events = self.parse_tempo_events(track)
                break

        for track in pattern:
            if len([True for x in track if isinstance(x, midi.NoteOnEvent)]) > 0:
                note_events = self.parse_note_events(track)
                break

        self.notes = self.merge_note_tempo_events(note_events, tempo_events)

    def merge_note_tempo_events(self, note_events, tempo_events):
        notes = collections.OrderedDict()
        i = 0
        last_t = 0

        for tempo_event in tempo_events:
            for n,note_event in enumerate(note_events[i:]):
                if note_event['tick'] >= tempo_event['before_tick'] and tempo_event['before_tick'] is not None:
                    i += n
                    break

                if note_event['tick'] < tempo_event['after_tick']:
                    continue

                pitch = note_event['pitch']
                if pitch < 1 or pitch > 5:
                    continue

                t = (tempo_event['after_time'] + (note_event['tick'] - tempo_event['after_tick'])*tempo_event['tick_dur']) * 1000

                dur = note_event['dur']*tempo_event['tick_dur']

                if t >= last_t:
                    last_t = t

                    if t not in notes:
                        notes[t] = []
                    notes[t].append({
                        'pitch': pitch,
                        'dur': dur,
                        'time': t})

        return notes


    def parse_note_events(self, track):
        events = []
        for i,event in enumerate(track):
            if isinstance(event, midi.NoteOnEvent):
                if event.get_velocity() > 0:
                    pitch = event.get_pitch()
                    tick = event.tick
                    dur = 0
                    for e in track[i+1:]:
                        if e.get_pitch() == pitch and e.get_velocity() == 0:
                            dur = e.tick - tick
                            break
                    events.append({
                        'tick': tick,
                        'pitch': pitch+1-self.top_note,
                        'dur': dur})
        return events

    def parse_tempo_events(self, track):
        bpms = []
        for event in track:
            if isinstance(event, midi.SetTempoEvent):
                bpms.append({
                    'after_tick': event.tick,
                    'bpm': event.get_bpm(),
                    'tick_dur': 60/(event.get_bpm()*self.resolution)})

        for i,bpm in enumerate(bpms):
            if i == len(bpms)-1:
                bpm['before_tick'] = None
            else:
                bpm['before_tick'] = bpms[i+1]['after_tick']

            if i != 0:
                bpm['after_time'] = bpms[i-1]['after_time'] + ((bpm['after_tick'] - bpms[i-1]['after_tick']) * bpms[i-1]['tick_dur'])
            else:
                bpm['after_time'] = 0

        return bpms

    def isPlaying(self):
        if not pygame.mixer.get_init():
            return False

        return pygame.mixer.music.get_busy()
