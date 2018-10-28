/**********************************************************
*
*    Filename     midi.cpp
*    Author       A Cowan
*    Start date   02/12/2017
*    Project      PULSE
*    Description  Class representing a midi
*                 file to be decoded
*
**********************************************************/

#include "Arduino.h"
#include "midi.h"

midi::midi()    //default constructor
{
}

midi::midi(char* inFilename)
{
	numberOfTracks = 0;
	for (int i = 0; i < 4; i++)
	{
		midiTrack[i].loaded = false;
		midiTrack[i].contents = 0;
		midiTrack[i].highestNote = 0;
		midiTrack[i].lowestNote = 255;
		midiTrack[i].presetLowestNote = 0;
	}
	filename = inFilename;
  trackNoteCount = 0;
}

/**
*	\brief Decodes the midi header
*
*	Reads the header of the midi file. Verifies the formatting is correct,
*	and extracts the numvber of tracks and tick division.
*
*	\return An integer representing status
**/
uint8_t midi::decodeHeader(void)
{
	File midiFile = SD.open(filename);

	if (!midiFile)
	{
		Serial.print("Unable to open ");
		Serial.println(filename);
		return E_UNABLE_TO_OPEN_MIDI;
	}

	//Midi file has been opened - now we can read it!

	char header[15];

	for (int i = 0; i < 14; i++)
		header[i] = midiFile.read();    //read in header
	header[14] = 0;                   //null terminate

	if (header[13] == 255)
	{
		Serial.println("File ended too early");
		return E_MIDI_TOO_FEW_BYTES;
	}

	midiFile.close();

	if (strcmp(header, "MThd") != 0)
	{
		Serial.println("Midi file didn't start with header");
		return E_MIDI_MISSING_MTHD;
	}

	uint16_t fileFormat = (((uint16_t)header[8] << 8) & 0xFF00) | (header[9] & 0x00FF);
	numberOfTracks = (((uint16_t)header[10] << 8) & 0xFF00) | (header[11] & 0x00FF);
	uint16_t hdrDivision = (((uint16_t)header[12] << 8) & 0xFF00) | (header[13] & 0x00FF);

	if (fileFormat != 1)
	{
		Serial.print("Invalid file format (");
		Serial.print(fileFormat);
		Serial.println(")");
		return E_MIDI_INVALID_FORMAT;
	}

	if (hdrDivision & 0x8000)
	{
		Serial.println("SMTPE timestamped midi not supported");
		return E_MIDI_SMTPE_TIME_MODE;
	}

	for (int i = 0; i < 4; i++)
		midiTrack[i].division = hdrDivision;

	return E_SUCCESS;
}

/**
*	\brief Scan of a file's midi tracks
*
*	Sequentially scans each track of a midi file. This calculates the
*	contents of each track (tempo/notes/lyrics/etc), and the highest
*	and lowest notes in the track.
*
*	\return An integer representing status
**/
uint8_t midi::scanTracks()
{
	midiTrack[0].loaded = false;
	midiTrack[0].startOffset = 14;  //due to file header size
	uint8_t result = decodeMidiTrack(0, false, false);
	if (result != E_SUCCESS)
		return result;

	for (int i = 1; i < numberOfTracks; i++)
	{
		midiTrack[i].loaded = false;
		midiTrack[i].startOffset = midiTrack[i - 1].startOffset + midiTrack[i - 1].trackLength;
		result = decodeMidiTrack(i, false, false);
		if (result != E_SUCCESS)
			return result;
	}
	return E_SUCCESS;
}

/**
*	\brief Populate the note calendar
*
*	Determines the first track including notes (using previous scan results),
*	and populates the notes calendar with these notes (with timestamps in
*	ticks). If 'presetNote' is zero, the range of lowest to highest note MUST
*	be 5, and that is mapped straight to the 5 buttons. If 'presetNote' is
*	set, notes of 'presetNote' to 'presetNote+5' are mapped to the buttons
*	(and all others are ignored)
*
*	\return An integer representing status
**/
uint8_t midi::populateCalendar(uint8_t presetNote)
{
	for (int i = 0; i < 4; i++)
	{
		if (midiTrack[i].contents & 0x02)  //look for the first track containing notes
		{
			midiTrack[i].presetLowestNote = presetNote;
			return decodeMidiTrack(i, true, false);
		}
	}
	Serial.println("Didn't find any notes tracks");
	return E_MIDI_NO_NOTES_FOUND;
}

/**
*	\brief Calculate timings for the note calendar
*
*	Determines the first track including tempos (using previous scan results),
*	and uses this to convert the timestamps in the notes calendar from 'ticks'
*	into milliseconds. It also applies a fixed offset to all notes (midi_offset),
*	which is uses to represent an offset between OGG and MIDI file.
*
*	\return An integer representing status
**/
uint8_t midi::computeTimings(uint16_t in_midi_offset)
{
	midi_offset = in_midi_offset;   //offset to be applied to all notes
	for (int i = 0; i < 4; i++)
	{
		if (midiTrack[i].contents & 0x01)  //look for the first track containing tempo information
		{
			return decodeMidiTrack(i, false, true);
		}
	}
	Serial.println("Didn't find any tempo tracks");
	return E_MIDI_NO_TEMPO_FOUND;
}

/**
*	\brief Decodes the midi file
*
*	Using previosuly computed file offsets, runs through a single midi track. Depending
*	on the flags used, this can just report what it sees (notes, tempos etc), it can
*	populate the note calendar, or it can apply tempo corrections to an existing note calendar.
*
*	\return An integer representing status
**/
uint8_t midi::decodeMidiTrack(uint8_t trackID, bool populateNotes, bool computeTimings)
{
	uint8_t notesRange;

	if (populateNotes)
	{
		if (midiTrack[trackID].highestNote == 0)
		{
			Serial.println("Notes track not yet parsed");
			return E_MIDI_RUN_PARSE_FIRST;
		}
		else
		{
    /*
			Serial.print("Found notes from ");
			Serial.print(midiTrack[trackID].lowestNote);
			Serial.print(" to ");
			Serial.print(midiTrack[trackID].highestNote);
			Serial.print(". Preset is ");
			Serial.println(midiTrack[trackID].presetLowestNote);
      */
		}

		notesRange = midiTrack[trackID].highestNote - midiTrack[trackID].lowestNote;

		noteList.reset();
	}

	note newNote;                 //new note, to be added to the noteList
	uint32_t timeNoteDown[5];     //used to record when each button was initially pressed
	uint16_t posInCalendar[5];    //used to record where the last press of each button was (so the duration can be put in)

	uint32_t lastTickCalc = 0;
	uint32_t lastTimeCalc = 0;
	uint16_t nextPosUpdte = 0;
	uint32_t lastTempo = 0;

	char header[8];
	File midiFile = SD.open(filename);

	if (!midiFile)
	{
		Serial.print("Unable to open ");
		Serial.println(filename);
		return E_UNABLE_TO_OPEN_MIDI;
	}

	if (!midiFile.seek(midiTrack[trackID].startOffset))
	{
		Serial.println("Unable to seek");
		return E_MIDI_UNABLE_TO_SEEK;
	}

	for (int i = 0; i < 8; i++)
		header[i] = midiFile.read();    //read in header
	header[8] = 0;                    //null terminate

	if (strcmp(header, "MTrk") != 0)
	{
		Serial.println("Formatting error in track");
		Serial.print(header);
		Serial.println(" != MTrk");
		return E_MIDI_MISSING_MTRK;
	}

	//header parsed successfully; we know we now have a load of events to process.
	//events start at startPos+8, and have a length of eventsLength.

	//Each event consists of:
	//  A delta time (up to 4 bytes)
	//    Last byte has MSB of 0, all others 1
	//  A command byte (MSB of 1)
	//  Data byte(s) (LSB of 0)

	uint32_t eventsLength = ((header[4] << 24) | (header[5] << 16) | (header[6] << 8) | (header[7]));
	uint32_t bytesParsed = 0;
	uint8_t lastCommandByte = 0;		//used for running mode

	uint32_t deltaTicks = 0;
	uint32_t elapsedTicks = 0;

	midiTrack[trackID].noteCount = 0;
	midiTrack[trackID].tempoChanges = 0;
	midiTrack[trackID].contents = CONTENTS_NONE;

	while (bytesParsed < eventsLength)
	{
		//everything starts with a variable length delta time; read that first

		int delta1 = midiFile.read();

		if (delta1 == -1)
		{
			Serial.println("File ended too early");
			Serial.print("Received ");
			Serial.print(bytesParsed);
			Serial.print(" expected ");
			Serial.print(eventsLength);
			Serial.print(" :");
			Serial.println(midiFile.read());
			return E_MIDI_TOO_FEW_BYTES;
		}

		if ((delta1 & 0x80) != 0)  //bytes to follow
		{
			char delta2 = midiFile.read();
			if ((delta2 & 0x80) != 0)  //bytes to follow
			{
				char delta3 = midiFile.read();
				if ((delta3 & 0x80) != 0)  //bytes to follow
				{
					char delta4 = midiFile.read();
					deltaTicks = ((delta1 & 0x7F) << 21) | ((delta2 & 0x7F) << 14) | ((delta3 & 0x7F) << 7) | (delta4 & 0x7F);  //four byte delta
					bytesParsed += 4;
				}
				else
				{
					deltaTicks = ((delta1 & 0x7F) << 14) | ((delta2 & 0x7F) << 7) | (delta3 & 0x7F);  //three byte delta
					bytesParsed += 3;
				}
			}
			else
			{
				deltaTicks = ((delta1 & 0x7F) << 7) | (delta2 & 0x7F);  //two byte delta
				bytesParsed += 2;
			}
		}
		else
		{
			deltaTicks = (delta1 & 0x7F);  //one byte delta
			bytesParsed++;
		}

		elapsedTicks += deltaTicks;	//total time into the track (in ticks)

		char commandByte = midiFile.read();
		bytesParsed++;

		switch (commandByte)
		{
		case 0xFF:    // Meta event
		{
			char typeByte = midiFile.read();
			char dataLength = midiFile.read();
			char data[dataLength];
			for (int i = 0; i < dataLength; i++)
			{
				data[i] = midiFile.read();
			}

			bytesParsed += (dataLength + 2);

			switch (typeByte)
			{
			case 0x51:    //set tempo
			{
				if (dataLength != 3)
				{
					Serial.println("Invalid tempo length");
					return E_MIDI_TEMPO_FORMAT;
				}

				uint32_t newTempo = (data[0] << 16) | (data[1] << 8) | (data[2] & 0x7F);
				midiTrack[trackID].contents |= CONTENTS_TEMPO;
				midiTrack[trackID].tempoChanges++;
				if (computeTimings)
				{
					if (elapsedTicks == 0)
					{
						lastTempo = newTempo; //nothing to calculate at first, so just remember it
					}
					else
					{
						// The division factor is decoded in the header , and is stored in all tracks.
						// For all my files, it seems to always be using 480 ticks per beat
						// And tempo is microseconds per beat

						// Therefore microseconds = (tempo * deltaTicks) / 480

						uint32_t tickToMicros = (lastTempo / (midiTrack[trackID].division / 10));  //we keep one divide by 10 for later to save rounding

						for (int i = (noteList.tempoSetPtr + 1); i < noteList.totalNotes; i++)
						{
							uint32_t nextNoteTick = noteList.noteArray[i].timestamp;		//the tick count of the next note to be converted

							if (nextNoteTick > elapsedTicks)  //can't calculate ticks for future tempos
								break;

							uint32_t deltaTick = nextNoteTick - lastTickCalc;

							/*	Serial.print(" Note[");
								Serial.print(i);
								Serial.print("]: ");
								Serial.print(noteList.noteArray[i].timestamp);
								Serial.print(" ticks (");
								Serial.print(deltaTick);
								Serial.print(" delta): ");*/

							noteList.noteArray[i].timestamp = lastTimeCalc + ((deltaTick * tickToMicros) / 10000);
							noteList.noteArray[i].duration = ((noteList.noteArray[i].duration * tickToMicros) / 10000);
							noteList.tempoSetPtr++;

							/*	Serial.print(noteList.noteArray[i].timestamp);
								Serial.println(" ms");*/
						}
						lastTimeCalc = lastTimeCalc + (((elapsedTicks - lastTickCalc) * tickToMicros) / 10000);
						lastTickCalc = elapsedTicks;
						lastTempo = newTempo;
					}
				}
				break;
			}
			case 0x58:  //set time signature
			{
				// Don't think I have to do anything with this - fingers crossed!
				data[0];  //numerator
				data[1];  //denominator (2=quarter, 3=eighth etc)
				data[2];  //number of ticks in metronome click
				data[3];  //number of 32nd notes to a quarter note
				break;
			}
			case 0x2F:
			{
				midiTrack[trackID].contents |= CONTENTS_END;  //  we found the end
				break;
			}
			case 0x01:  //arbitary text
			{
				//don't care about text
				break;
			}
			case 0x03:    //sequence name
			{
				//don't care what they called the track
				break;
			}
			case 0x05:  //lyrics
			{
				midiTrack[trackID].contents |= CONTENTS_LYRICS;  //  track contains lyrics
				break;
			}
			default:
			{
				Serial.print("Unknown meta event, type ");
				Serial.println(typeByte, HEX);
				midiTrack[trackID].contents |= CONTENTS_OTHER;  //  track contains something unknown
				break;
			}
			}
			break;
		}
		default:
		{
			if (((commandByte & 0xE0) == 0x80) || ((commandByte & 0x80) == 0x00))
			{
				// Either note on, note off, or no command byte (running mode)
				uint8_t noteNum, noteVel;

				if ((commandByte & 0x80) == 0x00) //MSB of command byte = 0 means we're in running mode
				{
					//therefore assume the same command byte as before
					noteNum = commandByte;
					noteVel = midiFile.read();
					commandByte = lastCommandByte;
					bytesParsed++;
				}
				else   //otherwise we're in regular mode
				{
					noteNum = midiFile.read();
					noteVel = midiFile.read();
					bytesParsed += 2;
				}

				midiTrack[trackID].contents |= CONTENTS_NOTES;  //  track contains notes

				uint8_t buttonID = 0;
				bool buttonValid;

				if (populateNotes)  //if presetLowestNote is set, then just process buttons in the range 0 to 4 (ignore the rest)
				{                   //if it isn't set and we're in auto mode, we have no idea what to do with more than 5...

					if (midiTrack[trackID].presetLowestNote > 0)
					{
						if (noteNum >= midiTrack[trackID].presetLowestNote)
						{
							buttonID = noteNum - midiTrack[trackID].presetLowestNote;
							if (buttonID < 5)
								buttonValid = true;
							else
								buttonValid = false;
						}
						else
						{
							buttonValid = false;
						}
					}
					else if ((noteNum >= midiTrack[trackID].lowestNote) && (noteNum <= midiTrack[trackID].highestNote))
					{
						buttonID = noteNum - midiTrack[trackID].lowestNote;

						if (notesRange == 3)
						{
							buttonID++;   //shift 0-3 ranges to the middle
						}
						buttonValid = true;
					}
					else	//in auto mode, but there's a big range
					{
						Serial.print("Unexpected note value of ");
						Serial.print(noteNum);
						Serial.print(", range is ");
						Serial.print(midiTrack[trackID].lowestNote);
						Serial.print(" to ");
						Serial.print(midiTrack[trackID].highestNote);
						Serial.println(", and auto mode is enabled.");
						return E_MIDI_INVALID_NOTE;
					}
				}
				if (((commandByte & 0xF0) == 0x80) | (noteVel == 0))  //note off
				{
					if ((populateNotes) && (buttonValid))
					{
						//we add a note to the noteList when we receive the off signal.
						//for simultaneous notes, there are multiple off signals - therefore
						//we may have already added the note (and set configured back to false)
						if (newNote.configured)
						{
							if (noteList.totalNotes == CALENDAR_MAX_SIZE)
							{
								Serial.println("Too many notes in track");
								return E_MIDI_TOO_MANY_NOTES;
							}
              else
              {
                if (trackNoteCount < noteList.totalNotes)
                  trackNoteCount = noteList.totalNotes;
              }
							newNote.duration = elapsedTicks - timeNoteDown[buttonID];
							noteList.addNote(newNote);
							newNote.reset();
						}
					}
				}
				else if (((commandByte & 0xF0) == 0x90)) //note on
				{
					if (noteNum > midiTrack[trackID].highestNote)
					{
						midiTrack[trackID].highestNote = noteNum;
					}
					if (noteNum < midiTrack[trackID].lowestNote)
					{
						midiTrack[trackID].lowestNote = noteNum;
					}

					if ((populateNotes) && (buttonValid))
					{
						if (elapsedTicks == newNote.timestamp)
						{
							//for simultaneous notes, keep adding events to the current note
						}
						else if (newNote.configured == false)
						{
							//we weren't already playing one, so start a new note
						}
						else
						{
							//we were playing a note which started at a different time
							Serial.println("Recieved a 'note on' while another was already on");

							Serial.print("Last note: ");
							Serial.print(newNote.timestamp);
							Serial.print(",");
							Serial.print(newNote.event);
							Serial.print(",");
							Serial.print(newNote.duration);
							Serial.print(",");
							Serial.print(newNote.configured);
							Serial.print("\n");

							Serial.print("Now note: ");
							Serial.print(elapsedTicks);
							Serial.print(",");
							Serial.print(noteNum);
							Serial.print("\n");
							return E_MIDI_OVERLAPPING_NOTES;
						}

						newNote.event = newNote.event | (0x01 << buttonID);
						newNote.timestamp = elapsedTicks;
						newNote.configured = true;
						timeNoteDown[buttonID] = elapsedTicks;
					}
				}
			}
			else
			{
				Serial.print("unknown event, type ");
				Serial.println(commandByte, HEX);
				midiTrack[trackID].contents |= CONTENTS_OTHER;  //  track contains something unknown
			}
			break;
		}
		}
		lastCommandByte = commandByte;
	}

	if ((populateNotes) && (newNote.configured))
	{
		Serial.println("Missing 'note off' at end");
		return E_MIDI_MISSING_NOTE_OFF;
	}

	if (computeTimings)   //we are computing timings, but the last tempo event was prior to the end
	{
		/*	        Serial.println("Computing remaining timings");
							Serial.print("Notes ");
							Serial.print(noteList.tempoSetPtr + 1);
							Serial.print(" to ");
							Serial.println(noteList.totalNotes);*/
		uint32_t tickToMicros = (lastTempo / (midiTrack[trackID].division / 10));  //we keep one divide by 10 for later to save rounding

		for (int i = (noteList.tempoSetPtr + 1); i < noteList.totalNotes; i++)
		{
			uint32_t nextNoteTick = noteList.noteArray[i].timestamp;    //the tick count of the next note to be converted

			uint32_t deltaTick = nextNoteTick - lastTickCalc;

			/*      Serial.print(" Note[");
				  Serial.print(i);
				  Serial.print("]: ");
				  Serial.print(noteList.noteArray[i].timestamp);
				  Serial.print(" ticks (");
				  Serial.print(deltaTick);
				  Serial.print(" delta): ");*/

			noteList.noteArray[i].timestamp = lastTimeCalc + ((deltaTick * tickToMicros) / 10000);
			noteList.noteArray[i].duration = ((noteList.noteArray[i].duration * tickToMicros) / 10000);
			noteList.tempoSetPtr++;

			/*      Serial.print(noteList.noteArray[i].timestamp);
				  Serial.println(" ms");*/
		}

		//apply start offset to all notes
		for (int i = 0; i < noteList.totalNotes; i++)
		{
			noteList.noteArray[i].timestamp += midi_offset;
		}
	}

	/*
	Serial.print("  Track ");
	Serial.print(trackID);
	Serial.print(" decoding complete: ");
	if (midiTrack[trackID].contents & CONTENTS_TEMPO)
	{
	Serial.print("TEMPO (");
	Serial.print(midiTrack[trackID].tempoChanges);
	Serial.print(") ");
	}
	if (midiTrack[trackID].contents & CONTENTS_NOTES)
	{
	Serial.print("NOTES (");
	Serial.print(midiTrack[trackID].noteCount);
	Serial.print(") ");
	}
	if (midiTrack[trackID].contents & CONTENTS_LYRICS)
	Serial.print("LYRICS ");
	if (midiTrack[trackID].contents & CONTENTS_OTHER)
	Serial.print("OTHER ");
	if (midiTrack[trackID].contents & CONTENTS_ERROR)
	Serial.print("ERROR ");

	Serial.println();
	*/

	midiFile.close();

	midiTrack[trackID].loaded = true;
	midiTrack[trackID].trackLength = eventsLength + 8; //include length of header

	return E_SUCCESS;
}



