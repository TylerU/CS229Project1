------------------------
sndconv
------------------------
Converts from a correctly formatted CS229 or AIFF sound file to the opposite type of sound file (or the same type).

By default, uses stdin for file input and stdout for converted file output.

Usage: $ sndconv [switches]
	Valid Switches:
		-l: Behave as part 1 and create a new file:
			Enter the filename of the sound file you would like to read. There is no output after the file name prompt. The program will exit when writing to the new file is complete. 
			The new file name will be [old file name].[new file extension]
				eg. "hello.cs229" will become "hello.cs229.aiff" and "hello.txt" formatted as a CS229 file becomes "hello.txt.aiff"
		-h: Display a short help screen to standard error, then terminate without performing any actions.
		-a: Output to AIFF no matter what the input format is.
		-c: Output to CS229 no matter what the input formt is.

Notes:
	If neither -a or -c switches are specified, the file will be written to the opposite format. If both switches are specified, the program will write to one format or the other (undefined).

	If the resulting file is an AIFF file, the following are assumed true:
		SSND offset == 0
		SSND BlockSize == 0

