#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gen_helpers.h"
#include "sound_info.h"

#define DEFAULT_SNDSHOW_WIDTH 80
#define DEFAULT_SNDSHOW_ZOOM 1
#define SHOW_ALL_CHANNELS 0
#define DONT_PRINT_THIS_SAMPLE_NUM -1
#define SAMPLE_NUMBER_WIDTH 9

#define ABS(x) ((x) < 0 ? -(x) : (x))

typedef struct {
	int just_show_help;
	int show_just_this_channel;
	int output_width;
	int zoom_factor;
} sndshow_switches;

/*
	Caution: Returns 0 for failure. Only use for applications in which 0 is not a valid result. 
*/
int get_int_from_str(char str[]){
	int result = strtol(str, NULL, 10);
	return result;
}

int get_max_value_in_num_bits(int n){
	int result = 1;
	/*return 2^(n-1) - 1;*/
	result <<= (n-1);
	result -= 1;
	return result;
}

int get_min_value_in_num_bits(int n){
	/*return -(2^(n-1));*/
	int result = 1;
	result <<= (n-1);
	result = -result;
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

void print_sample_num_if_necessary(FILE* out, int num_to_print){
	if(num_to_print != DONT_PRINT_THIS_SAMPLE_NUM){
		fprintf(out, "%9d", num_to_print);
	}
	else{
		fprintf(out, "         ");
	}
}

void print_spaces(FILE* out, int num){
	int i;
	for(i = 0; i < num; i++)
		fprintf(out, " ");
}

int print_channel(FILE* out, sound_reading channel_val, int num_to_print, int bit_depth, int total_width){
	int num_chars_for_each_side = (total_width-12)/2;
	double block_worth_pos = (double) get_max_value_in_num_bits(bit_depth) / num_chars_for_each_side;
	double block_worth_neg = (double) get_min_value_in_num_bits(bit_depth) / num_chars_for_each_side;

	print_sample_num_if_necessary(out, num_to_print);

	fprintf(out, "|");/*begin*/
	if(channel_val < 0){
		int i;
		for(i = -(num_chars_for_each_side-1); i <= 0; i++){
			if(channel_val <= block_worth_neg * -i + block_worth_neg/2.0){
				fprintf(out, "-");
			}
			else{
				fprintf(out, " ");
			}
		}
	}
	else{
		print_spaces(out, num_chars_for_each_side);
	}
	fprintf(out, "|");/*mid*/
	if(channel_val > 0){
		int i;
		for(i = 0; i < num_chars_for_each_side; i++){
			if(channel_val >= block_worth_pos * i + block_worth_pos/2.0){
				fprintf(out, "-");
			}
			else{
				fprintf(out, " ");
			}
		}
	}
	else{
		print_spaces(out, num_chars_for_each_side);
	}
	fprintf(out, "|\n");/*end*/
	return OK;
}

int print_sample(FILE* out, sample_node *sample, int sample_num, sound_file *data, sndshow_switches options){
	sound_reading *cur = sample->channel_data;
	int i;
	for(i = 0; i < data->channels; i++){
		int print_number = i==0 || options.show_just_this_channel!=SHOW_ALL_CHANNELS;
		int num_to_print = print_number ? sample_num : DONT_PRINT_THIS_SAMPLE_NUM;
		if(options.show_just_this_channel == SHOW_ALL_CHANNELS || (i+1) == options.show_just_this_channel){
			print_channel(out, *(cur+i), num_to_print, data->bit_depth, options.output_width);
		}
	}
	return OK;
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

	return OK;
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
	return OK;
}

int verify_switches(sndshow_switches switches, sound_file *data){
	if(switches.output_width < 20 || switches.output_width % 2 != 0){
		return INVALID_ARGUMENTS;
	}

	if(switches.show_just_this_channel < 1 || switches.show_just_this_channel > data->channels){
		return INVALID_ARGUMENTS;
	}

	if(switches.zoom_factor <= 0){
		return INVALID_ARGUMENTS;
	}

	return OK;
}

int sndshow(int argc, char *argv[]){
	int result = OK;
	sndshow_switches switches = parse_sndshow_switches(argc, argv);


	if(switches.just_show_help){
		print_readme(SNDSHOW_README_FILE, stderr);
		result = OK;
	}
	else {
		sound_file *data = create_empty_sound_file_data();	
		result = get_sound_info(stdin, data);
		if(result == OK){
			result = verify_switches(switches, data);
			if(result == OK){
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