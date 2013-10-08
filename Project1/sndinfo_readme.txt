------------------------
sndinfo
------------------------
Displays useful information about given sound file(s) including sample rate, format, bit depth, channels, samples, and sound duration. 

Usage: $ sndinfo [switches] [file] [file] ...
	Output is sent to stdout, error info to stderr. 
	Valid Switches:
		-h: Display this help screen to standard error, then terminate without reading files.
		-l: Ignore file arguments. Prompt user for file to read. Display resulting info. 
	If neither switch is used, sndinfo will read each file and print its information to stdout, separated by a line of dashes.
		

