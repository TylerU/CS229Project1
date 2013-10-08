#include <stdio.h>
#include "gen_helpers.h"

void write_file_to_file(FILE *src, FILE *dest){
	if(src != NULL && dest != NULL){
		char c;
		while( (c=fgetc(src)) != EOF){
			fputc(c, dest);
		}
	}
}

void append_file_to_file(char src[], FILE *dest_file){
	FILE *source_file = fopen(src, "r");
	write_file_to_file(source_file, dest_file);
}

int main(){
	FILE *readme = fopen(README_FILE, "w");
	append_file_to_file(README_HEADER_FILE, readme);
	append_file_to_file(SNDINFO_README_FILE, readme);
	append_file_to_file(SNDCONV_README_FILE, readme);
	append_file_to_file(SNDCAT_README_FILE, readme);
	append_file_to_file(SNDCUT_README_FILE, readme);
	append_file_to_file(SNDSHOW_README_FILE, readme);
	fclose(readme);
}