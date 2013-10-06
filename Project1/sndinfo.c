#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "gen_helpers.h"
#include "sound_info.h"
				
void print_dashes(FILE* out){
	fprintf(out, "------------------------------------------------------------\n");
}


void format_output(sound_file *file_data, char* file_name, FILE *out){
	char *sound_duration = get_sound_duration_string(file_data);
	fprintf(out, "Filename: %s\n", file_name);
	fprintf(out, "Format: %s\n", file_type_to_string(file_data->type));
	fprintf(out, "Sample Rate: %0.0Lf\n", file_data->sample_rate);
	fprintf(out, "Bit Depth: %d\n", file_data->bit_depth);
	fprintf(out, "Channels: %d\n", file_data->channels);
	fprintf(out, "Samples: %d\n", file_data->samples);
	fprintf(out, "Duration: %s\n", sound_duration);
	print_dashes(out);
	free(sound_duration);
}

int read_and_display_file(FILE* in, FILE* out, char file_name[]){
	int result;
	sound_file *file_data = create_empty_sound_file_data();
	
	if(in)
		result = get_sound_info(in, file_data);
	else
		result = COULDNT_OPEN_FILE;
	
	if(result == OK){
		format_output(file_data, file_name, out);
	}
	else{
		fprintf(out, "Filename: %s\nUnable to read this sound file. Check stderr for more information\n", file_name);
	}

	free_sound_file_data(file_data);
	return result;
}

int open_and_read_and_display_file(char file_name[], FILE* out){
	FILE* in = fopen(file_name, "rb");
	int result;
	
	result = read_and_display_file(in, out, file_name);

	return result;
}

int prompt_read_and_display_file(FILE* prompt_from, FILE* prompt_to){
	int result = 0;
	char file_name[DEFAULT_BUFFER_LENGTH] = "INVALID_FILE_NAME";

	fprintf(prompt_to, "Enter the pathname of a sound file:\n");
	fgets(file_name, DEFAULT_BUFFER_LENGTH, prompt_from);
	file_name[strlen(file_name)-1]=0;

	print_dashes(prompt_to);

	result = open_and_read_and_display_file(file_name, prompt_to);

	return result;
}


int sndinfo(int argc, char* argv[]){
	int result = OK;
	
	basic_switches switches = parse_switches(argc, argv);

	if(switches.just_show_help){
		print_readme(SNDINFO_README_FILE, stdout);
	}
	else if(switches.act_like_part1){
		result = prompt_read_and_display_file(stdin, stdout);

		print_if_error(result, "User specified");/*bad, hacky, but it doesn't really matter*/
	}
	else if(switches.first_non_switch == argc){/* No files specified */
		print_dashes(stdout);
		result = read_and_display_file(stdin, stdout, "(standard input)");
		print_if_error(result, "(standard input)");
	}
	else {
		int i;
		print_dashes(stdout);
		for(i = switches.first_non_switch; i < argc; i++){
			char * file_name;
			file_name = argv[i];
			result = open_and_read_and_display_file(file_name, stdout);
			print_if_error(result, file_name);
		}
	}

	return result;
}



int main(int argc, char* argv[]){
	int result;

	result = sndinfo(argc, argv);

	if(WINDOWS){
		system("pause");
	}

	return result != 1;
}
