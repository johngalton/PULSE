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
logger = logging.getLogger(__name__)

class Song:

    def __init__(self, location):
        self.location = location
        if not os.path.isdir(location):
            logger.error("Song directory does not exist")
        if not os.path.exists(location+'/song.ini') or os.path.exists(location+'/guitar.ogg') or os.path.exists(location+'/notes.mid'):
            logger.error("One or more files missing from directory")

        self.read_ini()
        self.read_midi()

    def calc_note_length(self, rate):
        # Rate in ms
        for t,items in self.notes.iteritems():
            for note in items:
                x = int(round(note['dur']/(rate/1000.0)))
                x = 1 if x < 1 else x
                note['len'] = x


    def read_ini(self):
        config = ConfigParser.ConfigParser()
        config.read(self.location + '/song.ini')
        self.artist = config.get('song', 'artist')
        self.name = config.get('song', 'name')
        self.delay = config.getfloat('song', 'delay')/1000 # Seconds
        self.top_note = config.getint('song', 'top_note')

    def read_midi(self):

        pattern = midi.read_midifile(self.location+'/notes.mid')
        self.resolution = float(pattern.resolution)
        pattern.make_ticks_abs()

        tempo_events = self.parse_tempo_events(pattern[0])
        note_events = self.parse_note_events(pattern[1])

        self.notes = self.merge_note_tempo_events(note_events, tempo_events)

    def merge_note_tempo_events(self, note_events, tempo_events):
        notes = collections.OrderedDict()
        i = 0

        for tempo_event in tempo_events:
            for n,note_event in enumerate(note_events[i:]):

                if note_event['tick'] >= tempo_event['before_tick'] and tempo_event['before_tick'] is not None:
                    i += n
                    break

                pitch = note_event['pitch']
                if pitch < 1 or pitch > 5:
                    continue

                t = tempo_event['after_time'] + (note_event['tick'] - tempo_event['after_tick'])*tempo_event['tick_dur']

                dur = note_event['dur']*tempo_event['tick_dur']

                if t not in notes:
                    notes[t] = []
                notes[t].append({
                    'pitch': pitch,
                    'dur': dur})

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
