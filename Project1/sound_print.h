#ifndef SOUND_PRINT_H
#define SOUND_PRINT_H
#include "gen_helpers.h"
#include <stdlib.h>

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

int get_max_value_in_num_bits(int n);

int get_min_value_in_num_bits(int n);

void append_spaces(char** out, int num);

void append_sample_num_if_necessary(char **str, int num_to_print);

int get_sound_sample_string(char *str, sound_reading channel_val, int num_to_print, int bit_depth, int total_width);

#endif