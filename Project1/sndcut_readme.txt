------------------------
sndcut
------------------------
Reads a sound file from the standard input stream, then writes the sound file to standard output in the same format after removing all samples specified as arguments. 

Usage: $ sndcut [switches] [low..high] [low..high] ...
 	Valid Switches:
		-h: Display a short help screen to standard error, then terminate without performing any actions.
	High..low pairs must specify valid sample indices where 0 <= low <= samples in the file - 2 and low <= high <= samples in the file - 1
	Sample call:
		$ sndcut 100..1000 4000..5000 < test.cs229 > test1.cs229

