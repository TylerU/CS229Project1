------------------------
sndinfo
------------------------
Displays useful information about a given sound file including sample rate, format, bit depth, channels, samples, and sound duration. 

Capable of reading correctly formatted CS229 and AIFF files. If said files contain misformats, a descriptive error message will be sent to stderr. 
	Supported bit depths: 8, 16, 32 bits
	Maximum channels: 32
	Sample rate must be an integer

Usage: 	make sndinfo (or make)
		Then enter the filename of the sound file you would like to read. Output is sent to stdout, error info to stderr. 

Notes:
	Assumes the given file can be opened successfully. Will print an error if not. 
	Stores the entire sound sample data in memory. This may crash systems with low RAM and a large sound file. 

