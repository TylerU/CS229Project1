#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef SOUND_INFO
	#include "sound_info.h"
	#define SOUND_INFO
#endif

#ifndef GEN_HELPERS
	#include "gen_helpers.h"
	#define GEN_HELPERS
#endif


int main(){
	int result = 0;
	FILE *in;
	char file_name[DEFAULT_BUFFER_LENGTH] = "Not Implemented";
	sound_file *file_data = create_empty_sound_file_data(); 
	if(DEBUG){
		strcpy(file_name, "zep.aiff");
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
