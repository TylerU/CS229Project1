#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "gen_helpers.h"
#include "sound_info.h"
#include "sound_writing.h"

const char* error_descriptions[] = {"UNEXPECTED_EOF or Input Error","OK","UNRECOGNIZED_FILE_FORMAT","WRONG_NUMBER_OF_SOUND_READINGS","NOT_ENOUGH_SAMPLES",
	"Empty sample data line or not enough valid channels data", "Invalid or unspecified bit depth", "Invalid or unspecified channels value", "Invalid or unspecified sample rate",
	"Encountered invalid header identifier or beginning of sample data without proper specifier", "Not an error, encountered the beginning of start data",
	"Invalid header value or unexpected end of file", "Unable to allocate more memory", "Couldn't open the given file", "Invalid arguments", 
	"Unable to concatenate the given files. Ensure that the files have the same number of channels, sample rate, and bit depth", "Invalid block size specified"};

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

basic_switches parse_switches(int argc, char* argv[]){
	basic_switches switches = {0, 0, 0, 0, argc};
	int i;

	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
			case 'l':
				switches.act_like_part1 = 1;
				break;
			case 'h':
				switches.just_show_help = 1;
				break;
			case 'c':
				switches.output_as_cs229 = 1;
				break;
			case 'a':
				switches.output_as_aiff = 1;
				break;
			default:
				/* Ignore this parameter */
				/* Print error?? */
				break;
			}
		}
		else{
			switches.first_non_switch = i;
			break;
		}
	}

	return switches;
}

file_type get_file_type_restriction_from_switches(basic_switches switches){
	file_type reqd_out = UNRECOGNIZED;
	if(switches.output_as_aiff && switches.output_as_cs229){
		reqd_out = CS229;/*default, I guess*/
	}
	else if(switches.output_as_aiff){
		reqd_out = AIFF;
	}
	else if(switches.output_as_cs229){
		reqd_out = CS229;
	}
	else{
		reqd_out = UNRECOGNIZED;
	}
	return reqd_out;
}

file_type get_opposite_type(file_type type){
	if(type == AIFF){
		return CS229;
	}
	else if(type == CS229){
		return AIFF;
	}
	else{
		return UNRECOGNIZED;
	}
}

int read_and_write_result_using_stdio(file_type output_restriction){
	int result = 0;
	sound_file *file_data = create_empty_sound_file_data(); 

	result = get_sound_info(stdin, file_data);

	if(result == OK){
		file_type new_type = get_opposite_type(file_data->type);
		if(output_restriction != UNRECOGNIZED){
			if(output_restriction == AIFF)
				new_type = AIFF;
			else
				new_type = CS229;
		}

		result = write_to_file_type(stdout, file_data, new_type);
	}
	
	free_sound_file_data(file_data);
	return result;
}

void print_special_error(char err[]){
	fprintf(stderr, err);
}