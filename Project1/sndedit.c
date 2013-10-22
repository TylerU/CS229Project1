#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include "gen_helpers.h"
#include "sound_info.h"
#include "sound_writing.h"
#include "sound_print.h"

typedef int boolean;
#define true 1
#define false 0

typedef struct {
	int total_rows;
	int total_cols;
	int editor_cols;
	int info_cols;

	int row;
	int default_col;
	sound_file *data;
	sample_node *sample_at_top;
	int cur_sample_number;
	int top_channel_index;
	char *file_name;
	char *header;

	boolean allow_mark;
	boolean allow_copy;
	boolean allow_cut;
	boolean allow_insert;
	boolean allow_save;

	boolean quit;

	int num_marked;
	int num_buffered;
} sndedit_state;


char *create_sndedit_header(sound_file *data, char *file_name){
	int len = strlen(file_name) + 40; /*enough room for the file name as well as our junk on the end*/
	char *header = (char*) malloc(sizeof(char) * len);
	sprintf(header, "%s (%s)", file_name, file_type_to_string(data->type));
	return header;
}


sndedit_state create_sndedit_state(sound_file *data, char *file_name){
	sndedit_state state;
	getmaxyx(stdscr, state.total_rows, state.total_cols);
	state.info_cols = 20;	
	state.editor_cols = state.total_cols - state.info_cols;
	state.row = state.total_rows / 2;
	state.default_col = 9 + (state.editor_cols - 9) / 2;

	state.data = data;
	state.file_name = file_name;
	state.sample_at_top = data->sample_data_head;
	state.cur_sample_number = 0;
	state.top_channel_index = 0;
	state.header = create_sndedit_header(data, file_name);
	state.allow_mark = true;
	state.allow_copy = false;
	state.allow_cut = false;
	state.allow_insert = false;
	state.allow_save = false;
	state.quit = false;

	state.num_marked = 0;
	state.num_buffered = 0;
	return state;
}

int init_ncurses(){
	initscr();
	noecho();           /* don't print input */
	cbreak();           /* don't buffer input */
	keypad(stdscr, 1);  /* Get arrows */
	return GOOD;
}

int deinit_ncurses(){
	endwin();
	return GOOD;
}

int output_num_chars(int c, int n, int row, int col){
	move(row, col);
	char *str = (char*) malloc(sizeof(char) * (n+1));
	memset(str, c, n);
	str[n] = 0;
	printw(str);
	free(str);
}

int print_centered_header(sndedit_state *state){
	mvprintw(0, state->total_cols / 2 - strlen(state->header) / 2, state->header);
}

int print_key_and_description_if_true(int row, int col, char *keys, char* description, boolean print){
	if(print){
		mvprintw(row, col, "%3s: %s", keys, description);
	}
}

int print_sidebar(sndedit_state *state){
	int sidebar_start = state->total_cols - 20 + 1;
	char *sound_duration = get_sound_duration_string(state->data);
	int row = 2;

	mvprintw(row++, sidebar_start, "Sample Rate: %0.0Lf\n", state->data->sample_rate);
	mvprintw(row++, sidebar_start, "  Bit Depth: %d\n", state->data->bit_depth);
	mvprintw(row++, sidebar_start, "   Channels: %d\n", state->data->channels);
	mvprintw(row++, sidebar_start, "Samples: %d\n", state->data->samples);
	mvprintw(row++, sidebar_start, " Length: %s\n", sound_duration);
	output_num_chars('=', 20, row++, sidebar_start -1);
	free(sound_duration);

	print_key_and_description_if_true(row++, sidebar_start, "m", "mark / unmark", state->allow_mark);
	print_key_and_description_if_true(row++, sidebar_start, "c", "copy", state->allow_copy);
	print_key_and_description_if_true(row++, sidebar_start, "x", "cut", state->allow_cut);
	print_key_and_description_if_true(row++, sidebar_start, "^", "insert before", state->allow_insert);
	print_key_and_description_if_true(row++, sidebar_start, "v", "insert after", state->allow_insert);
	print_key_and_description_if_true(row++, sidebar_start, "s", "save", state->allow_save);
	print_key_and_description_if_true(row++, sidebar_start, "q", "quit", true);
	row++;

	mvprintw(row++, sidebar_start, " Movement");
	mvprintw(row++, sidebar_start, " up/dn");
	mvprintw(row++, sidebar_start, " pgup/pgdn");
	mvprintw(row++, sidebar_start, " g: goto sample");
	row++;

	output_num_chars('=', 20, row++, sidebar_start -1);	
	mvprintw(row++, sidebar_start, "   Marked: %d", state->num_marked);
	mvprintw(row++, sidebar_start, " Buffered: %d", state->num_buffered);
}

int print_samples(sndedit_state *state){
	sample_node *cur = state->sample_at_top;
	int cur_channel_index = state->top_channel_index;
	int sample_num = state->cur_sample_number;
	int i;
	for(i = 2; i < state->total_rows; i++){
		sound_reading sample = cur->channel_data[cur_channel_index];
		char *cur_str = (char *) malloc(sizeof(char) * state->editor_cols);

		get_sound_sample_string(cur_str, sample, cur_channel_index == 0 ? sample_num : DONT_PRINT_THIS_SAMPLE_NUM, state->data->bit_depth, state->editor_cols);
		mvprintw(i, 0, cur_str);
		
		cur_channel_index++;
		if(cur_channel_index >= state->data->channels){
			cur=cur->next;
			cur_channel_index = 0;
			sample_num++;
			if(!cur)
				break;
		}
		free(cur_str);
	}
}

int print_main_screen(sndedit_state *state){
	print_centered_header(state);
	output_num_chars('=', state->total_cols, 1, 0);
	print_sidebar(state);
	print_samples(state);
}

int handle_input(sndedit_state *state, int in){
	switch (in) {
		case 'q': 
			state->quit = true;
			break;
		case KEY_UP:
			if (state->row>2) state->row--;
			break;
		case KEY_DOWN:
			if (state->row<state->total_rows-1) state->row++;
			break;
		case KEY_PPAGE:
			break;
		case KEY_NPAGE:
			break;
		default:
			break;
	}
}


int fill_data_from_file_arg(int argc, char *argv[], sound_file *data){
	FILE *in;
	int result;

	if(argc < 2){
		return INVALID_ARGUMENTS;
	}

	in = fopen(argv[1], "r");
	
	if(in)
		result = get_sound_info(in, data);
	else
		result = COULDNT_OPEN_FILE;

	return result;
}

int main(int argc, char *argv[])
{
	init_ncurses();
	int result;
	sound_file *data = create_empty_sound_file_data();
	result = fill_data_from_file_arg(argc, argv, data);

	/*Check for errors */
	if(result == 1){
		sndedit_state state = create_sndedit_state(data, argv[1]);

		while(!state.quit) {
			print_main_screen(&state);
			move(state.row, state.default_col);
			refresh();

			int c = getch();
			handle_input(&state, c);
		}
	}
	else{
		print_if_error(result, "Given file");
	}

	free_sound_file_data(data);

	deinit_ncurses();
	return 0;
}