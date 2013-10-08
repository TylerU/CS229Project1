------------------------
sndshow
------------------------
Prints an ASCII art representation of a valid sound file read from standard input.
	Each line is a separate sound reading for a specific channel.
	The numbered lines mark the beginning of a new sample. 
	
Usage: $ sndshow [switches]
	Valid Switches:
		-h: Display a short help screen to standard error, then terminate without performing any actions.
		-c c : Show the output only for channel c, for 1 <= c <= # of channels in sound file.
		-w w : Specify the total output width, in number of characters. If not specified, the default is w = 80. Note that w must be even so that the number of characters for representing positive values is equal to the number of characters for representing negative values. This program supports values down to w = 20.
		-z n : Zoom out by a factor of n. If not specified, the default is n = 1. The value to "plot" should be the largest magnitude value over n consecutive samples, and the number of lines of output should decrease by about a factor of n.

