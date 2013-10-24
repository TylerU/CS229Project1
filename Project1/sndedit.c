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
	int top_channel_index;
	char *file_name;
	char *header;


	boolean quit;
	boolean changes_made;
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
	state.default_col = 9 + (state.editor_cols - 9) / 2;
	state.row = 2;

	state.data = data;
	state.file_name = file_name;
	state.sample_at_top = data->sample_data_head;
	state.top_channel_index = 0;
	state.header = create_sndedit_header(data, file_name);
	state.changes_made = false;

	state.quit = false;
	return state;
}


int get_sndedit_state_cur_row(sndedit_state *state){
	return state->row;
}

int get_sndedit_state_top_sample_number(sndedit_state *state){

}

int get_sndedit_state_top_channel_index(sndedit_state *state){

}

sample_node *get_sndedit_state_sample_at_top(sndedit_state *state){

}

boolean get_sndedit_state_allow_mark(sndedit_state *state){
	return get_sndedit_state_num_marked(state) < 1;
}

boolean get_sndedit_state_allow_unmark(sndedit_state *state){
	return get_sndedit_state_num_marked(state) > 0;
}

boolean get_sndedit_state_allow_copy(sndedit_state *state){
	return get_sndedit_state_num_marked(state) > 0;
}

boolean get_sndedit_state_allow_cut(sndedit_state *state){
	return get_sndedit_state_num_marked(state) > 0;
}

boolean get_sndedit_state_allow_insert(sndedit_state *state){
	return get_sndedit_state_num_buffered(state) > 0;
}

boolean get_sndedit_state_allow_save(sndedit_state *state){
	return state->changes_made;
}

int get_sndedit_state_num_marked(sndedit_state *state){

}

int get_sndedit_state_num_buffered(sndedit_state *state){

}

boolean get_sndedit_state_show_marked(sndedit_state *state){
	return get_sndedit_state_num_marked(state) > 0;
}

boolean get_sndedit_state_show_buffered(sndedit_state *state){
	return get_sndedit_state_num_buffered(state) > 0;
}

void set_sndedit_state_changed(sndedit_state *state, boolean val){
	state->changes_made = val;
}


boolean get_sndedit_state_can_move_down(sndedit_state *state){
	//return get_sndedit_state_top_channel_index(state) < state->data->samples;
} 

boolean get_sndedit_state_can_move_up(sndedit_state *state){
	return get_sndedit_state_top_channel_index(state) > 0;
}

int move_sndedit_state_down(sndedit_state *state){
	if(get_sndedit_state_can_move_down(state)){

	}
	else{
		/* Ignore command */
	}
}

int move_sndedit_state_up(sndedit_state *state){
	if(get_sndedit_state_can_move_up(state)){

	}
	else{
		/* Ignore command */
	}
}

int move_sndedit_state_up_page(sndedit_state *state){
	if(get_sndedit_state_can_move_up(state)){

	}
	else{
		/* Ignore command */
	}
}

int move_sndedit_state_down_page(sndedit_state *state){
	if(get_sndedit_state_can_move_down(state)){

	}
	else{
		/* Ignore command */
	}
}

int initiate_copy(sndedit_state *state){
	if(get_sndedit_state_allow_copy(state)){

	}
	else{
		/* Ignore command */
	}
}

int initiate_cut(sndedit_state *state){
	if(get_sndedit_state_allow_cut(state)){

	}
	else{
		/* Ignore command */
	}
}

int initiate_insert(sndedit_state *state, boolean after){
	if(get_sndedit_state_allow_insert(state)){

	}
	else{
		/* Ignore command */
	}
}

int initiate_goto(sndedit_state *state){

}

int attempt_goto_sample_num(sndedit_state *state, int sample){

}

int initiate_mark(sndedit_state *state){
	if(get_sndedit_state_allow_mark(state)){

	}
	else if(get_sndedit_state_allow_unmark(state)){

	}
	else{
		return INVALID_STATE;
	}
}

int save_state(sndedit_state *state){
	if(get_sndedit_state_allow_save(state)){

	}
	else{
		/* Ignore this command */
	}
	return GOOD;
}

int sndedit_quit(sndedit_state *state){
	state->quit = true;
	return GOOD;
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
	

	print_key_and_description_if_true(row, sidebar_start, "m", "mark", get_sndedit_state_allow_mark(state));
	print_key_and_description_if_true(row, sidebar_start, "m", "unmark", get_sndedit_state_allow_unmark(state));
	row++;
	print_key_and_description_if_true(row++, sidebar_start, "c", "copy", get_sndedit_state_allow_copy(state));
	print_key_and_description_if_true(row++, sidebar_start, "x", "cut", get_sndedit_state_allow_cut(state));
	print_key_and_description_if_true(row++, sidebar_start, "^", "insert before", get_sndedit_state_allow_insert(state));
	print_key_and_description_if_true(row++, sidebar_start, "v", "insert after", get_sndedit_state_allow_insert(state));
	print_key_and_description_if_true(row++, sidebar_start, "s", "save", get_sndedit_state_allow_save(state));
	print_key_and_description_if_true(row++, sidebar_start, "q", "quit", true);
	row++;

	mvprintw(row++, sidebar_start, " Movement");
	mvprintw(row++, sidebar_start, " up/dn");
	mvprintw(row++, sidebar_start, " pgup/pgdn");
	mvprintw(row++, sidebar_start, " g: goto sample");
	row++;

	output_num_chars('=', 20, row++, sidebar_start -1);	

	if(get_sndedit_state_show_marked(state))
		mvprintw(row++, sidebar_start, "   Marked: %d", get_sndedit_state_num_marked(state));
	if(get_sndedit_state_show_buffered(state))
		mvprintw(row++, sidebar_start, " Buffered: %d", get_sndedit_state_num_buffered(state));	

	free(sound_duration);
}

int print_samples(sndedit_state *state){
	sample_node *cur = get_sndedit_state_sample_at_top(state);
	int cur_channel_index = get_sndedit_state_top_channel_index(state);
	int sample_num = get_sndedit_state_top_sample_number(state);
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
	int result;
	switch (in) {
		case 'm':
			result = initiate_mark(state);
			break;
		case 'c':
			result = initiate_copy(state);
			break;
		case 'x':
			result = initiate_cut(state);
			break;
		case '^':
			result = initiate_insert(state, false);
			break;
		case 'v':
			result = initiate_insert(state, true);
			break;
		case 's':
			result = save_state(state);
			break;
		case 'q': 
			result = sndedit_quit(state);
			break;
		case KEY_UP:
			result = move_sndedit_state_down(state);
			break;
		case KEY_DOWN:
			result = move_sndedit_state_up(state);
			break;
		case KEY_PPAGE:
			result = move_sndedit_state_up_page(state);
			break;
		case KEY_NPAGE:
			result = move_sndedit_state_down_page(state)
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
			move(get_sndedit_state_cur_row(&state), state.default_col);
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