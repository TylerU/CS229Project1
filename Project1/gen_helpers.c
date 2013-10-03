#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifndef GEN_HELPERS
	#include "gen_helpers.h"
	#define GEN_HELPERS
#endif

const char* error_descriptions[] = {"UNEXPECTED_EOF or Input Error","OK","UNRECOGNIZED_FILE_FORMAT","WRONG_NUMBER_OF_SOUND_READINGS","NOT_ENOUGH_SAMPLES",
	"Empty sample data line or not enough valid channels data", "Invalid or unspecified bit depth", "Invalid or unspecified channels value", "Invalid or unspecified sample rate",
	"Encountered invalid header identifier or beginning of sample data without proper specifier", "Not an error, encountered the beginning of start data",
	"Invalid header value or unexpected end of file", "Unable to allocate more memory", "Couldn't open the given file"};

int flip_endian(char *dest, int size){
	char* temp = (char*)malloc(size);
	int i;
	memcpy(temp, dest, size);
	for(i = 0; i < size; i++){
		*(dest + i) = *(temp+size-1-i);
	}

	free(temp);
	return OK;
}

int find_string_and_get_following(FILE* in, char *find, char *last){
	int c;
	while((c = fgetc(in)) != EOF && c == *(find) && *find != '\0'){
		find++;
	}
	ungetc(c, in);
	*last = c;
	if((*find)=='\0'){
		return OK;
	}
	else if (c==EOF){
		return UNEXPECTED_EOF;
	}
	else {
		return 0;
	}
}

int find_string_and_ensure_following_whitespace(FILE *in, char *find){
	char last_char;
	int result;
	if((result = find_string_and_get_following(in, find, &last_char)) && isspace(last_char)){
		return OK;
	}
	else{
		return result;
	}
}

int print_readme(char file_name[], FILE* out){
	FILE *readme = fopen(file_name, "r");
	if(readme){
		char c;
		while( (c = fgetc(readme)) != EOF ){
			fputc(c, out);
		}
	}
}

void print_if_error(int err_code, char file_name[]){
	if(err_code != OK){
		fprintf(stderr, "Error on file: %s. Error code %d: %s\n", file_name, err_code, error_descriptions[err_code]);
	}
}