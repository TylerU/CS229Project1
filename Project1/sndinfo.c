#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef GEN_HELPERS
	#include "gen_helpers.h"
	#define GEN_HELPERS
#endif

#ifndef SOUND_INFO
	#include "sound_info.h"
	#define SOUND_INFO
#endif

void format_output(sound_file *file_data, char* file_name){
	printf("------------------------------------------------------------\n");
	printf("Filename: %s\n", file_name);
	printf("Format: %s\n", file_type_to_string(file_data->type));
	printf("Sample Rate: %0Lg\n", file_data->sample_rate);
	printf("Bit Depth: %d\n", file_data->bit_depth);
	printf("Channels: %d\n", file_data->channels);
	printf("Samples: %d\n", file_data->samples);
	printf("Duration: %s\n", get_sound_duration_string(file_data));
	printf("------------------------------------------------------------\n");
}

int main(int argc, char* argv[]){
	int result = 0;
	FILE *in;
	char file_name[DEFAULT_BUFFER_LENGTH] = "Not Implemented";
	sound_file *file_data = create_empty_sound_file_data(); 
	if(DEBUG){
		strcpy(file_name, "hello.aiff");
	}
	else{
		printf("Enter the pathname of a sound file:\n");
		fgets(file_name, DEFAULT_BUFFER_LENGTH, stdin);
		file_name[strlen(file_name)-1]=0;
	}

	in = fopen(file_name, "rb");
	if(in){
		result = get_sound_info(in, file_data);
		fclose(in);
	}
	else{
		fprintf(stderr, "Unable to open file\n");
	}

	if(result == OK){
		format_output(file_data, file_name);
	}
	else{
		fprintf(stderr, "Error Code %d: %s\n", result, error_descriptions[result]);
	}


	if(WINDOWS){
		system("pause");
	}

	return result;
}
