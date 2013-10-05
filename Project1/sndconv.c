#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "sound_info.h"
#include "gen_helpers.h"
#include "sound_writing.h"

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

int write_to_file_type(FILE* out, sound_file *file_data, file_type type){
	if(type == AIFF){
		return write_to_aiff(out, file_data);
	}
	else if(type == CS229){
		return write_to_cs229(out, file_data);
	}
	else{
		return UNRECOGNIZED_FILE_FORMAT;
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
		append_file_type_extension(new_type, file_name);
		outf = fopen(file_name , "wb");
		if(outf && new_type != UNRECOGNIZED)
			result = write_to_file_type(outf, file_data, new_type);
		else
			result = COULDNT_OPEN_FILE;
	}

	free_sound_file_data(file_data);
	return result;
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

int sndconv(int argc, char* argv[]){
	int result = OK;
	
	basic_switches switches = parse_switches(stdin, argc, argv);
	
	if(switches.just_show_help){
		print_readme(SNDCONV_README_FILE, stdout);
	}
	else if(switches.act_like_part1){
		result = act_like_part_1();
	}
	else {
		/* figure out what output type MUST be */
		file_type reqd_out = UNRECOGNIZED;
		if(switches.output_as_aiff && switches.output_as_cs229){
			result = INVALID_ARGUMENTS;
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
