#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "sound_info.h"
#include "gen_helpers.h"
#include "sound_writing.h"





int append_file_type_extension(file_type type, char *file_name){
	if(type == AIFF){
		strcat(file_name, ".aiff");
	}
	else if(type == CS229){
		 strcat(file_name, ".cs229");
	}
	else{
		return UNRECOGNIZED_FILE_FORMAT;
	}
	return OK;
}

int get_new_file_name(char file_name[], int n){
	char *res;
	printf("Enter the pathname of the other sound file:\n");
	res = fgets(file_name, n, stdin);
	if(res){
		file_name[strlen(file_name)-1] = 0;/* remove that last \n */
		return OK;
	}
	else{
		return UNEXPECTED_EOF;
	}
}

int act_like_part_1(){
	int result = 0;
	FILE *in;
	char file_name[DEFAULT_BUFFER_LENGTH] = "Not Implemented";
	sound_file *file_data = create_empty_sound_file_data(); 

	printf("Enter the pathname of a sound file:\n");
	fgets(file_name, DEFAULT_BUFFER_LENGTH, stdin);
	file_name[strlen(file_name)-1]=0;

	in = fopen(file_name, "rb");
	if(in){
		result = get_sound_info(in, file_data);
		fclose(in);
	}
	else{
		result = COULDNT_OPEN_FILE;
	}

	if(result == OK){
		FILE *outf;
		file_type new_type = get_opposite_type(file_data->type);
		result = get_new_file_name(file_name, DEFAULT_BUFFER_LENGTH);
		return_if_not_OK(result);
		outf = fopen(file_name , "wb");
		if(outf && new_type != UNRECOGNIZED)
			result = write_to_file_type(outf, file_data, new_type);
		else
			result = COULDNT_OPEN_FILE;
	}

	free_sound_file_data(file_data);
	return result;
}

int sndconv(int argc, char* argv[]){
	int result = OK;
	
	basic_switches switches = parse_switches(argc, argv);
	
	if(switches.just_show_help){
		print_readme(SNDCONV_README_FILE, stderr);
	}
	else if(switches.act_like_part1){
		result = act_like_part_1();
	}
	else {
		/* figure out what output type MUST be */
		file_type reqd_out = get_file_type_restriction_from_switches(switches);

		if(result == OK){
			result = read_and_write_result_using_stdio(reqd_out);
		}
	}

	print_if_error(result, "");

	return result;
}


int main(int argc, char* argv[]){
	int result;

	result = sndconv(argc, argv);

	if(WINDOWS){
		system("pause");
	}

	return result != 1;
}
