#include <stdio.h>
#include <stdlib.h>
#include "gen_helpers.h"
#include "sound_info.h"
#include "sound_writing.h"

typedef struct {
	int high;
	int low;
} highlow;

highlow parse_highlow_arg(char arg[], int *err){
	highlow range;
	int result = 0;

	result = sscanf(arg, "%d..%d", &range.low, &range.high);
	
	if(result == 2)
		*err = OK;
	else
		*err = INVALID_ARGUMENTS;

	return range;
}

highlow *parse_highlow_args(char *args[], int argc, int *err){
	int i;
	highlow *all_ranges = (highlow *)malloc(sizeof(highlow) * argc);
	*err = OK;

	if(all_ranges){
		for(i = 0; i < argc; i++){
			int result;
			*(all_ranges+i) = parse_highlow_arg(args[i], &result);
			if(result != OK){
				*err = result;
			}
		}
	}
	return all_ranges;
}

int should_be_cut(int index, highlow *ranges, int num_ranges){
	int i;
	for (i = 0; i < num_ranges; i++){
		highlow range = *(ranges + i);
		if(index >= range.low && index <= range.high){
			return 1;
		}
	}
	return 0;
}

/* And updates the samples count */
int strip_samples_in_ranges(sound_file *data, highlow *ranges, int num_ranges){
	sample_node *cur = data->sample_data_head;
	sample_node *last_valid = NULL;
	int index = 0;
	int valid_samples = 0;
	data->sample_data_head = NULL;

	while(cur != NULL){
		if(should_be_cut(index, ranges, num_ranges)){
			sample_node *last_cur = cur;
			cur = cur->next;
			free_sample_node(last_cur);
		}
		else{
			sample_node *next;
			valid_samples++;
			if(last_valid != NULL){
				last_valid->next = cur;
			}
			else{
				data->sample_data_head = cur;
			}

			last_valid = cur;
			next = cur->next;
			cur->next = NULL;
			cur = next;
		}
		index++;
	}
	data->samples = valid_samples;
	return OK;
}

int apply_sound_cut(sound_file *data, char *args[], int cuts_start, int cuts_end){
	int result;
	int num_ranges = cuts_end - cuts_start + 1;
	highlow *ranges = parse_highlow_args(&args[cuts_start], num_ranges, &result); 

	if(result == OK){
		strip_samples_in_ranges(data, ranges, num_ranges);
	}

	free(ranges);
	return result;
}

int sndcut(int argc, char *argv[]){
	int result = OK;
	basic_switches switches = parse_switches(argc, argv);
	
	if(switches.just_show_help){
		print_readme(SNDCUT_README_FILE, stderr);
		result = OK;
	}
	else {
		sound_file *data = create_empty_sound_file_data();	
		result = get_sound_info(stdin, data);
		if(result == OK){
			result = apply_sound_cut(data, argv, switches.first_non_switch, argc-1);
			if(result == OK){
				result = write_to_file_type(stdout, data, data->type);
			}
		}
		free_sound_file_data(data);
	}

	print_if_error(result, "");

	return result;
}

int main(int argc, char *argv[]){
	int result;

	result = sndcut(argc, argv);

	if(WINDOWS){
		system("pause");
	}

	return result != 1;
}