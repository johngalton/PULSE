import midi
import collections
import time
import vlc


top_note = 96

pattern = midi.read_midifile("../songs/Survivor - Eye of the tiger/notes.mid")

resolution = pattern.resolution

pattern.make_ticks_abs()

bpms = []

for event in pattern[0]:
    if isinstance(event, midi.SetTempoEvent):
        bpms.append({
            'after_tick': event.tick,
            'bpm': event.get_bpm(),
            'len': 60/(event.get_bpm()*resolution)})

for i,bpm in enumerate(bpms):
    if i == len(bpms)-1:
        bpm['before_tick'] = None
    else:
        bpm['before_tick'] = bpms[i+1]['after_tick']

    if i != 0:
        bpm['after_time'] = bpms[i-1]['after_time'] + ((bpm['after_tick'] - bpms[i-1]['after_tick']) * bpms[i-1]['len'])
    else:
        bpm['after_time'] = 0

events = []

for i,event in enumerate(pattern[1]):
    if isinstance(event, midi.NoteOnEvent):
        if event.get_velocity() > 0:
            pitch = event.get_pitch()
            tick = event.tick
            dur = 0
            for e in pattern[1][i+1:]:
                if e.get_pitch() == pitch and e.get_velocity() == 0:
                    dur = e.tick - tick
                    break
            events.append({
                'tick': tick,
                'pitch': pitch+1-top_note,
                'dur': dur})



notes = collections.OrderedDict()

i = 0

for bpmchange in bpms:

    for n,event in enumerate(events[i:]):
        if event['tick'] >= bpmchange['before_tick'] and bpmchange['before_tick'] is not None:
            i += n
            break
        pitch = event['pitch']
        if pitch < 1 or pitch > 5:
            continue
        t = bpmchange['after_time'] + (event['tick'] - bpmchange['after_tick'])*bpmchange['len']
        dur = event['dur']*bpmchange['len']
        if t not in notes:
            notes[t] = []
        notes[t].append({
            'pitch': pitch,
            'dur': dur})

delay = 1669.0/1000

for t,items in notes.iteritems():
    for note in items:
        x = int(round(note['dur']/0.015))
        x = 1 if x < 1 else x
        note['dur'] = x

#target = open('notes.txt', 'w')
#target.write(str(notes))

#def play_music():
#    pyglet.options['audio'] = ('openal', 'silent')
#    source = pyglet.media.load('../songs/Survivor - Eye of the tiger/guitar.ogg')
#    player = source.play()

#t = threading.Thread(target=play_music)
#t.start()
p = vlc.MediaPlayer('../songs/Survivor - Eye of the tiger/guitar.ogg')
p.play()

i = time.clock()
print delay
while i < delay:
    i = time.clock()
zero = i
last = 0
nxt = notes.popitem(last=False)
hist = [0,0,0,0,0]
while True:
    #while time.clock() - last < 0.015:
    #    continue
    #last = time.clock()
    i = time.clock() - zero
    if nxt[0] < i:
        for note in nxt[1]:
            hist[note['pitch']-1] += note['dur']
        nxt = notes.popitem(last=False)
        #string = ""
        #print "-"
        #for n,bla in enumerate(hist):
        #    if bla > 0:
        #        string += "# "
        #        hist[n] = 0
        #    else:
        #        string += "  "
        #print string

    if i - last >= 0.015:
        string = ""
        for n,bla in enumerate(hist):
            if bla > 0:
                string += "# "
                hist[n] -= 1
            else:
                string += "  "
        print string
        last = i
