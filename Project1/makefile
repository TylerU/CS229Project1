all: sndconv sndinfo sndcat sndcut sndshow sndedit

readme: gen_readme readme_header.txt sndcat_readme.txt sndconv_readme.txt sndinfo_readme.txt sndcut_readme.txt sndshow_readme.txt sndedit_readme.txt
	./gen_readme

clean:
	rm -f gen_helpers.o sndconv.o sndinfo.o sndcat.o sndcut.o sndshow.o sndedit.o sound_info.o sound_writing.o gen_readme.o sndconv sndinfo sndcat sndcut sndshow sndedit gen_readme

tarball: proj1.tar.gz

proj1.tar.gz: makefile README gen_helpers.c gen_helpers.h gen_readme.c sndconv.c sndinfo.c sndcat.c sound_info.c sound_info.h  sound_writing.c sound_writing.h sound_print.c sound_print.h gen_readme.c sndcut.c sndshow.c sndedit.c readme_header.txt sndcat_readme.txt sndconv_readme.txt sndinfo_readme.txt sndcut_readme.txt sndshow_readme.txt sndedit_readme.txt
	tar -czf proj1.tar.gz makefile README gen_helpers.c gen_helpers.h gen_readme.c sndconv.c sndinfo.c sndcat.c sound_info.c sound_info.h  sound_writing.c sound_writing.h sound_print.c sound_print.h gen_readme.c sndcut.c sndshow.c sndedit.c readme_header.txt sndcat_readme.txt sndconv_readme.txt sndinfo_readme.txt sndcut_readme.txt sndshow_readme.txt sndedit_readme.txt
	
sndinfo: gen_helpers.o sound_info.o sndinfo.o
	gcc -o sndinfo gen_helpers.o sound_info.o sound_writing.o sndinfo.o

sndconv: gen_helpers.o sound_info.o sound_writing.o sndconv.o
	gcc -o sndconv gen_helpers.o sound_info.o sound_writing.o sndconv.o

sndcat: gen_helpers.o sound_info.o sound_writing.o sndcat.o
	gcc -o sndcat gen_helpers.o sound_info.o sound_writing.o sndcat.o

sndcut: gen_helpers.o sound_info.o sound_writing.o sndcut.o
	gcc -o sndcut gen_helpers.o sound_info.o sound_writing.o sndcut.o

sndshow: gen_helpers.o sound_info.o sound_writing.o sound_print.o sndshow.o 
	gcc -o sndshow gen_helpers.o sound_info.o sound_writing.o sound_print.o sndshow.o

sndedit: gen_helpers.o sound_info.o sound_writing.o sound_print.o sndedit.o
	gcc -o sndedit gen_helpers.o sound_info.o sound_writing.o sound_print.o sndedit.o -lncurses

gen_readme: gen_readme.o gen_helpers.o
	gcc -o gen_readme gen_readme.o gen_helpers.o sound_info.o sound_writing.o

gen_readme.o: gen_readme.c gen_helpers.h
	gcc -c gen_readme.c

gen_helpers.o: gen_helpers.h sound_info.h sound_writing.h gen_helpers.c
	gcc -c gen_helpers.c

sndconv.o: gen_helpers.h sound_info.h sound_writing.h sndconv.c
	gcc -c sndconv.c

sndinfo.o: gen_helpers.h sound_info.h sndinfo.c
	gcc -c sndinfo.c

sndcat.o: gen_helpers.h sound_info.h sound_writing.h sndcat.c
	gcc -c sndcat.c

sndcut.o: gen_helpers.h sound_info.h sound_writing.h sndcut.c
	gcc -c sndcut.c
	
sndshow.o: gen_helpers.h sound_info.h sound_print.h sndshow.c
	gcc -c sndshow.c

sndedit.o: gen_helpers.h sound_info.h sound_writing.h sound_print.h sndedit.c
	gcc -c sndedit.c
	
sound_writing.o: gen_helpers.h sound_writing.h sound_writing.c
	gcc -c sound_writing.c

sound_info.o: gen_helpers.h sound_info.h sound_info.c
	gcc -c sound_info.c

sound_print.o: gen_helpers.h sound_print.h sound_print.c
	gcc -c sound_print.c