/*
PULSE scoring method

 - Button down triggers event
 - Interrogate calendar for current note (incorporating window each side)
    - Returns a note, or no_note
    - Window is on each side of the start - doesn't include duration (ie you can't join a long note midway)
 - Compare event to the returned note
    - If (BUTTON_STATE == NOTE STATE) then we're pressing the correct note: record hit
	- If (NOTE_STATE == 0) then we've pressed a button when one wasn't needed: record miss
    - If ((BUTTON_STATE & (~NOTE_STATE)) != 0x00) then we're pressing a button we shouldn't: record miss
	- Otherwise it must be a partial note (half of a double pressed) - do nothing and wait for the next button press event
  
  - Long holds are handled seperatly on a timer
*/
  
  
  bool initNoteChecking(uint16_t in_startWindowMs, uint16_t in_endWindowMs)
  {
	  startWindowMs = in_startWindowMs;
	  endWindowMs = in_endWindowMs;
	  lastNotePressed = 0;
  }
  
  
  // Add a 'lastNoteHit' value to the calendar class.
  // Will make searching much faster - you can start
  // at the first unhit one, and keep going until
  // noteTime > currentTime
  
  
  struct noteEvent
  {
	uint16_t noteNumber;	//the number in the song (used for tracking if we missed one, as this should go up one each time)
	uint8_t noteState;		//the colours/podiums involved
	uin32_t noteTime;		//when the note was centred (used for determining accuracy)
  }
  
  noteEvent currentNote checkForValidNote(uint32_t playTime)
  {
	//looks through the calender from lastNotePressed
	
	noteEvent selectedNote;
	
	for (int i = lastNotePressed; i < totalNotes; i++)
	{
		if ((playTime > (noteArray[i].timestamp - startWindow)) && (playTime < (noteArray[i].timestamp + endWindow)))
		{
			lastNotePressed = i;
			selectedNote.noteNumber = i;
			selectedNote.noteState = noteArray[i].event;
			noteState.noteTime = noteArray[i].timestamp;
			return selectedNote;
		}
		
		// If our position in the calendar is ahead of playback, no point in searching any further
		if (playTime > ((noteArray[i].timestamp + endWindow)))
		{
			break;
		}
	}
	
	//no note found	  
	selectedNote.noteNumber = 0;
	selectedNote.noteState = 0x00;
	selectedNote.noteTime = 0;
	return selectedNote;
  }
 