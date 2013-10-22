#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gen_helpers.h"
#include "sound_info.h"
#include "sound_print.h"


/*
	Caution: Returns 0 for failure. Only use for applications in which 0 is not a valid result. 
*/
int get_int_from_str(char str[]){
	int result = strtol(str, NULL, 10);
	return result;
}


/*copy pasted code. Fix if I have time*/
sndshow_switches parse_sndshow_switches(int argc, char* argv[]){
	sndshow_switches switches = {0, SHOW_ALL_CHANNELS, DEFAULT_SNDSHOW_WIDTH, DEFAULT_SNDSHOW_ZOOM};
	int i;

	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
			case 'h':
				switches.just_show_help = 1;
				break;
			case 'c':
				if(i + 1 < argc){
					int next = get_int_from_str(argv[i+1]);
					i++;
					switches.show_just_this_channel = next;
				}
				break;
			case 'w':
				if(i + 1 < argc){
					int next = get_int_from_str(argv[i+1]);
					i++;
					switches.output_width = next;
				}				
				break;
			case 'z':
				if(i + 1 < argc){
					int next = get_int_from_str(argv[i+1]);
					i++;
					switches.zoom_factor = next;
				}				
				break;
			default:
				/* Ignore this parameter */
				/* Print error?? */
				break;
			}
		}
	}

	return switches;
}


int print_sample(FILE* out, sample_node *sample, int sample_num, sound_file *data, sndshow_switches options){
	sound_reading *cur = sample->channel_data;
	int i;
	for(i = 0; i < data->channels; i++){
		int print_number = i==0 || options.show_just_this_channel!=SHOW_ALL_CHANNELS;
		int num_to_print = print_number ? sample_num : DONT_PRINT_THIS_SAMPLE_NUM;
		if(options.show_just_this_channel == SHOW_ALL_CHANNELS || (i+1) == options.show_just_this_channel){
			char *sample_string = (char*) malloc(sizeof(char) * options.output_width + 1);
			get_sound_sample_string(sample_string, *(cur+i), num_to_print, data->bit_depth, options.output_width);
			fprintf(out, "%s\n", sample_string);
			free(sample_string);
		}
	}
	return GOOD;
}

/*Creates a node whose channel values are the greatest magnitude between start node and the next size nodes, or until the last node*/
int create_aggregate_sample(sample_node **start_node, int size, sample_node *new_node, int num_channels, int *counter){
	int cur_node, cur_channel;
	/*Init to 0*/
	for(cur_channel = 0; cur_channel < num_channels; cur_channel++){
		*(new_node->channel_data + cur_channel) = 0;
	}

	/*Find max in the range*/
	for(cur_node = 0; cur_node < size && *start_node != NULL; (*counter)++, cur_node++, (*start_node)=(*start_node)->next){
		for(cur_channel = 0; cur_channel < num_channels; cur_channel++){
			sound_reading this_val = *((*start_node)->channel_data + cur_channel);
			sound_reading prev_max = *(new_node->channel_data + cur_channel);
			
			if(ABS(this_val) > ABS(prev_max)){
				*(new_node->channel_data + cur_channel) = this_val;
			}
		}
	}

	return GOOD;
}

int print_sound(FILE* out, sound_file *data, sndshow_switches options){
	sample_node *cur = data->sample_data_head;
	int sample_num = 0;
	while(cur != NULL){
		sample_node *to_print = create_sample_node(data->channels);
		int prev_sample_num = sample_num;
		create_aggregate_sample(&cur, options.zoom_factor, to_print, data->channels, &sample_num);
		print_sample(out, to_print, prev_sample_num, data, options);
		free_sample_node(to_print);
	}
	return GOOD;
}

int verify_switches(sndshow_switches switches, sound_file *data){
	if(switches.output_width < 20 || switches.output_width % 2 != 0){
		return INVALID_ARGUMENTS;
	}

	if((switches.show_just_this_channel != SHOW_ALL_CHANNELS && switches.show_just_this_channel < 1) || switches.show_just_this_channel > data->channels){
		return INVALID_ARGUMENTS;
	}

	if(switches.zoom_factor <= 0){
		return INVALID_ARGUMENTS;
	}

	return GOOD;
}

int sndshow(int argc, char *argv[]){
	int result = GOOD;
	sndshow_switches switches = parse_sndshow_switches(argc, argv);


	if(switches.just_show_help){
		print_readme(SNDSHOW_README_FILE, stderr);
		result = GOOD;
	}
	else {
		sound_file *data = create_empty_sound_file_data();	
		result = get_sound_info(stdin, data);
		if(result == GOOD){
			result = verify_switches(switches, data);
			if(result == GOOD){
				result = print_sound(stdout, data, switches);
			}
		}
	}

	print_if_error(result, "");

	return result;
}


int main(int argc, char *argv[]){
	int result;

	result = sndshow(argc, argv);

	if(WINDOWS){
		system("pause");
	}

	return result != 1;
}