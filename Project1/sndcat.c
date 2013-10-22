#include <stdio.h>
#include <stdlib.h>
#include "gen_helpers.h"
#include "sound_info.h"
#include "sound_writing.h"

int read_sound_file(char file_name[], sound_file *data){
	int result;
	FILE* in = fopen(file_name, "rb");

	if(in)
		result = get_sound_info(in, data);
	else
		result = COULDNT_OPEN_FILE;

	return result;
}

int init_to_null(sound_file **files, int num_files){
	int i;
	for(i = 0; i < num_files; i++){
		*(files + i) = NULL;
	}
}

int free_all_file_structs(sound_file **files, int num_files){
	int i;
	for(i = num_files-1; i >= 0; i--){/*Mind blowing revelation. I need to free these BACKWARDS. */
		free_sound_file_data(*(files + i));
	}
	free(files);
}

int are_files_valid_for_concatenation(sound_file **files, int num_files){
	/*ensure same sample rate, channes, bit depth*/
	int i; 
	sound_file *first_data = *files;
	for(i = 1; i < num_files; i++){
		sound_file *data = *(files + i);
		if(	data->channels    != first_data->channels || 
			data->sample_rate != first_data->sample_rate ||
			data->bit_depth   != first_data->bit_depth)
		{
			return UNABLE_TO_CONCATENATE_FILES;
		}
	}

	return GOOD;
}

sample_node *get_samples_tail(sound_file *file){
	sample_node *cur = file->sample_data_head;
	while(cur->next != NULL){
		cur = cur->next;
	}
	return cur;
}

int concat_files_in_first(sound_file **files, int num_files){
	sample_node *cur_tail = get_samples_tail(*files);
	int i;
	int total_samples = (*files)->samples;

	for(i = 1; i < num_files; i++){
		total_samples += (*(files+i))->samples;
		cur_tail->next = (*(files + i))->sample_data_head;
		if(cur_tail->next){
			cur_tail = get_samples_tail(*(files + i));	
		}
		else{
			/*Empty samples data | DO NOT advance the cur_tail pointer. We will just skip this file*/
		}
	}

	(*files)->samples = total_samples;
	return GOOD;
}

int read_all_files(char *file_names[], int start, int end, sound_file **datas){
	int i;
	for(i = start; i <= end; i++){
		int result = GOOD;
		int cur_index = i-start;
		sound_file **cur_data = (datas+cur_index);
		*cur_data = create_empty_sound_file_data();
		result = read_sound_file(file_names[i], *cur_data);
		if(result != GOOD){
			return result;
		}
	}
	return GOOD;
}

int read_files_concat_and_print_resulting_file(char *file_names[], int names_start, int names_end, FILE* out, file_type out_type){
	int result = GOOD;
	int num_files = names_end + 1 - names_start;
	sound_file **files_data = (sound_file **)malloc(sizeof(sound_file*) * num_files);
	if(files_data){
		init_to_null(files_data, num_files);

		/*read files*/
		result = read_all_files(file_names, names_start, names_end, files_data);
		if(result == GOOD){
			/*check for errors*/
			result = are_files_valid_for_concatenation(files_data, num_files);
			if(result == GOOD){
				/*concatenate in first file*/
				result = concat_files_in_first(files_data, num_files);
				
				if(result == GOOD){
					/*write to given output*/
					if(out_type == UNRECOGNIZED){
						print_special_error("You need to choose an output file type using -a or -c");
						result = INVALID_ARGUMENTS;
					}
					else{
						result = write_to_file_type(stdout, *files_data, out_type);
					}
				}
			}
		}
		/*free all those guys*/
		/*free_all_file_structs(files_data, num_files);*/
	}

	return result;
}

int sndcat(int argc, char *argv[]){
	int result = GOOD;
	
	basic_switches switches = parse_switches(argc, argv);
	
	if(switches.just_show_help){
		print_readme(SNDCAT_README_FILE, stderr);
		result = GOOD;
	}
	else if(switches.first_non_switch < argc){
		result = read_files_concat_and_print_resulting_file(argv, switches.first_non_switch, argc-1, stdout, get_file_type_restriction_from_switches(switches));
	}
	else if(switches.first_non_switch == argc){
		/*Treat standard input as the input file*/
		result = read_and_write_result_using_stdio(get_file_type_restriction_from_switches(switches));
	}

	print_if_error(result, "");

	return result;
}

int main(int argc, char *argv[]){
	int result;

	result = sndcat(argc, argv);

	if(WINDOWS){
		system("pause");
	}

	return result != 1;
}