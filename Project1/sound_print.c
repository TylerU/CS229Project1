#include "sound_print.h"
#include "gen_helpers.h"
#include <stdio.h>

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
void append_spaces(char** out, int num){
	int i;
	for(i = 0; i < num; i++){
		*out[0] = ' ';
		(*out)++;
	}
}


void append_sample_num_if_necessary(char **str, int num_to_print){
	if(num_to_print != DONT_PRINT_THIS_SAMPLE_NUM){
		sprintf(*str, "%9d", num_to_print);
	}
	else{
		sprintf(*str, "         ");
	}
	*str += 9;
}


int get_sound_sample_string(char *str, sound_reading channel_val, int num_to_print, int bit_depth, int total_width){
	int num_chars_for_each_side = (total_width-12)/2;
	double block_worth_pos = (double) get_max_value_in_num_bits(bit_depth) / num_chars_for_each_side;
	double block_worth_neg = (double) get_min_value_in_num_bits(bit_depth) / num_chars_for_each_side;

	append_sample_num_if_necessary(&str, num_to_print);

	*str = '|';/*begin*/
	str++;

	if(channel_val < 0){
		int i;
		for(i = -(num_chars_for_each_side-1); i <= 0; i++){
			if(channel_val <= block_worth_neg * -i + block_worth_neg/2.0){
				*str = '-';
				str++;
			}
			else{
				*str = ' ';
				str++;
			}
		}
	}
	else{
		append_spaces(&str, num_chars_for_each_side);
	}
	
	*str = '|';/*mid*/
	str++;
	
	if(channel_val > 0){
		int i;
		for(i = 0; i < num_chars_for_each_side; i++){
			if(channel_val >= block_worth_pos * i + block_worth_pos/2.0){
				*str = '-';
				str++;
			}
			else{
				*str = ' ';
				str++;
			}
		}
	}
	else{
		append_spaces(&str, num_chars_for_each_side);
	}

	*str = '|';/*end*/
	str++; 
	*str = 0;
	str++;

	return GOOD;
}