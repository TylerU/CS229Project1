README

Author:
Tyler Uhlenkamp

Included:
All source files (.c, .h)
This README and associated .txt files for building it and printing help messages.
A makefile
	Make commands:
		make clean
		make readme
		make [name of program]
		make tarball

General notes:
	These programs are capable of reading ONLY correctly formatted CS229 and AIFF files. If said files contain misformats, a descriptive error message will be sent to stderr. 
		Supported bit depths: 8, 16, 32 bits
		Maximum channels: 32
		Sample rate must be an integer
	
	These programs assume the given file(s) can be opened successfully. Will print an error if not. 
	
	These programs store the entire sound sample data in memory. This may crash systems with low RAM and a large sound file. 

	All errors are printed to standard error.
Project #1 Part #2

