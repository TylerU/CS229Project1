------------------------
sndconv
------------------------
Converts from a correctly formatted CS229 or AIFF sound file to the opposite type of sound file (CS229 -> AIFF, AIFF -> CS229)

Capable of converting correctly formatted CS229 and AIFF files. If said files contain misformats, a descriptive error message will be sent to stderr. 
	Supported bit depths: 8, 16, 32 bits
	Maximum channels: 32
	Sample rate must be an integer

Usage: 	make sndconv (or make)
		Then enter the filename of the sound file you would like to read. There is no output after the file name prompt. The program will exit when writing to the new file is complete. 
		The new file name will be [old file name].[new file extension]
			eg. "hello.cs229" will become "hello.cs229.aiff" and "hello.txt" formatted as a CS229 file becomes "hello.txt.aiff"

Notes:
	If the resulting file is an AIFF file, the following are assumed true:
		SSND offset == 0
		SSND BlockSize == 0
	Stores the entire sound sample data in memory. This may crash systems with low RAM and a large sound file. 

