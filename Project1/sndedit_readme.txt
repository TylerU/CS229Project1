------------------------
sndedit
------------------------
Prints an ASCII art representation of a valid sound file and allows interaction with the sound data.
	Each line is a separate sound reading for a specific channel.
	The numbered lines mark the beginning of a new sample. 
	
Usage: $ sndedit file_name
	Keyboard commands:
		Up and Down arrow keys: 
			Move the selection pointer up or down one exactly sample. 
		
		Page up and page down
			Move the selection pointer up or down one page, roughly. 
		
		g:
			Initiate a goto command. This allows the user to enter a valid sample number which will be immediately jumped to. Just enter a number and press 'enter' 
			Note: 
				If an invalid sample number is entered, nothing will happen.

		m:
			Initiate marking mode. This will beging marking samples at the current sample. All samples between the initially marked sample and the selection cursor will be considered 'marked'. 
		
		c:
			Copy. Copies all the marked samples into a buffer. These samples can later be pasted at any other place in the sound file!
		
		x:
			Cut. Copies all the marked samples into a buffer and removes them from the sound. 
		
		^:
			Paste above. Places all samples in the buffer directly above the selected sample. This command can be repeated multiple times for MAXIMUM FUN.

		v:
			Paste below. Places all samples in the buffer directly below the selected sample. This command can be repeated multiple times for MAXIMUM FUN.

		s:
			Save changes. No changes are actually committed to the sound file until this command is executed.

		q: 
			Quit without saving. 