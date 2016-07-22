import midi
import collections
import time
import vlc

import song


s = song.Song('../songs/Survivor - Eye of the tiger')
s.calc_note_length(25)

notes = s.notes
delay = s.delay

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
            hist[note['pitch']-1] += note['len']
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

    if i - last >= 0.025:
        string = ""
        for n,bla in enumerate(hist):
            if bla > 0:
                string += "# "
                hist[n] -= 1
            else:
                string += "  "
        print string
        last = i
