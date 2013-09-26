all: sndconv sndinfo

clean:
	rm -f gen_helpers.o sndconv.o sndinfo.o sound_info.o sndconv sndinfo

tarball: proj1.tar.gz

proj1.tar.gz: makefile gen_helpers.c gen_helpers.h sndconv.c sndinfo.c sound_info.c sound_info.h 
	tar -czf proj1.tar.gz makefile README gen_helpers.c gen_helpers.h sndconv.c sndinfo.c sound_info.c sound_info.h 
	
sndinfo: gen_helpers.o sound_info.o sndinfo.o
	gcc -o sndinfo gen_helpers.o sound_info.o sndinfo.o

sndconv: gen_helpers.o sound_info.o sndconv.o
	gcc -o sndconv gen_helpers.o sound_info.o sndconv.o

gen_helpers.o: gen_helpers.c gen_helpers.h
	gcc -c gen_helpers.c

sndconv.o: gen_helpers.h gen_helpers.c sound_info.h sound_info.c sndconv.c
	gcc -c sndconv.c

sndinfo.o: gen_helpers.h gen_helpers.c sound_info.h sound_info.c sndinfo.c
	gcc -c sndinfo.c

sound_info.o: gen_helpers.c gen_helpers.h sound_info.c
	gcc -c sound_info.c
