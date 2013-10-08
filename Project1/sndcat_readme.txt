------------------------
sndcat
------------------------
Concatenates all sound files passed in as arguments and writes a single file to standard out. If no files are specified in arguments, stdin is treated as an input file. 

Usage: $ sndcat [switches] [file1] [file2] ...
	Valid Switches:
		-h: Display a short help screen to standard error, then terminate without performing any actions.
		-a: Output to AIFF no matter what the input format is.
		-c: Output to CS229 no matter what the input formt is.
		ONE OF THE PREVIOUS 2 SWITCHES MUST BE USED. 
	Files can be either AIFF or CS229 files.
	
Notes:
	If neither -a or -c switches are specified, the program will throw an error.
	 If both switches are specified, the program will write to one format or the other (undefined).

