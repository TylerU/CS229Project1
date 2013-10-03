#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "sound_info.h"
#include "gen_helpers.h"
#include "sound_writing.h"


int sndconv(int argc, char* argv[]){
	int result;
	int i;

	int act_like_part1 = 0;
	int just_print_help = 0;
	int output_as_aiff = 0;
	int output_as_cs229 = 0;
	
	for(i = 1; i < argc; i++){

	}

	return result;
}


int main(int argc, char* argv[]){
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
	}

	if(result == OK){
		if(file_data->type == CS229){
			FILE *outf = fopen( strcat(file_name, ".aiff") , "wb");
			write_to_aiff(outf, file_data);
		}
		else if (file_data->type == AIFF){
			FILE *outf = fopen( strcat(file_name, ".cs229") , "wb");
			write_to_cs229(outf, file_data);
		}
		else{

		}
	}
	else{
		fprintf(stderr, "Error Code %d: %s\n", result, error_descriptions[result]);
	}


	if(WINDOWS){
		system("pause");
	}

	return 0;
}
