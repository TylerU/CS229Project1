#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include "gen_helpers.h"
#include "sound_info.h"
#include "sound_writing.h"
#include "sound_print.h"

#define GOTO_STR_SIZE 30
#define LOG(M, A, B) fprintf(stderr, (M), (A), (B))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef int boolean;
#define true 1
#define false 0


typedef struct {
	int index;
	sample_node *sample;
} sndedit_node;

typedef struct {
	int size;
	sample_node *start;
	sample_node *end;
} sample_buffer;


typedef struct {
	int total_rows;
	int total_cols;
	int editor_cols;
	int info_cols;

	int row;
	int default_col;

	sound_file *data;
	sndedit_node top;
	sndedit_node *marked;
	sample_buffer *buffer;
	int top_channel;
	char *file_name;
	char *header;

	boolean goto_mode;
	char goto_str[GOTO_STR_SIZE];
	int goto_index;

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
	state.top.sample = data->sample_data_head;
	state.top.index = 0;
	state.top_channel = 0;
	state.header = create_sndedit_header(data, file_name);
	state.changes_made = false;

	state.buffer = NULL;
	state.marked = NULL;

	state.goto_mode = false;
	state.quit = false;
	return state;
}

sample_node *get_sndedit_state_sample_at_top(sndedit_state *state){
	return state->top.sample;
}

sndedit_node get_sndedit_state_selected_sample(sndedit_state *state){
	sndedit_node cur;
	int cur_row = (get_sndedit_state_cur_row(state) - 2);
	int channels = state->data->channels;
	int numSamples = cur_row / channels + (cur_row % channels == 0 ? 0 : 1);/* HELP */
	cur.index = get_sndedit_state_top_sample_number(state) + numSamples;
	cur.sample = get_sndedit_state_sample_at_top(state);
	int i;
	for(i = 0; i < numSamples; i++){
		cur.sample = cur.sample->next;
	}
	return cur;
}

sample_node* get_sample_by_index(sample_node *head, int index){
	int i;
	for(i = 0; i < index; i++){
		head = head->next;
	}
	return head;
}

int get_sndedit_state_cur_row(sndedit_state *state){
	return state->row;
}

int get_sndedit_state_top_sample_number(sndedit_state *state){
	return state->top.index;
}

int get_sndedit_state_top_channel_index(sndedit_state *state){
	return state->top_channel;
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
	if(state->marked){
		sndedit_node selected = get_sndedit_state_selected_sample(state);
		return ABS(selected.index - state->marked->index) + 1;
	}
	else{
		return 0;
	}
}

int get_sndedit_state_num_buffered(sndedit_state *state){
	if(state->buffer){
		return state->buffer->size;
	}
}

boolean get_sndedit_state_show_marked(sndedit_state *state){
	return get_sndedit_state_num_marked(state) > 0;
}

boolean get_sndedit_state_show_buffered(sndedit_state *state){
	return get_sndedit_state_num_buffered(state) > 0;
}

void set_sndedit_state_changed(sndedit_state *state, boolean val){
	state->changes_made = val;
	int samples = 0;
	sample_node *cur = state->data->sample_data_head;
	while(cur){
		samples++;
		cur=cur->next;
	}
	state->data->samples = samples;
}


boolean get_sndedit_state_can_move_down(sndedit_state *state, int num){
	return get_sndedit_state_selected_sample(state).index < state->data->samples - num;
} 

boolean get_sndedit_state_can_move_up(sndedit_state *state, int num){
	return get_sndedit_state_selected_sample(state).index >= num;
}

int get_total_editor_rows(sndedit_state *state){
	return state->total_rows - 2;
}
int set_sndedit_selected_sample(sndedit_state *state, sample_node *node, int index){
	/* Remember:
		Update row
		
		Update top sample
			Top sample will just become the one before it if we are at the top and want to go up
			Otherwise, we either just move the cursor down (if the sample is on screen)
			Or move the screen down just enough to show the selected sample

		Weird cases:
			#channels > screen height
			Lets just make the sample appear at the top of the screen (but then we can never see the rest of the channels :(
	*/
	int total_rows = get_total_editor_rows(state);
	int cur_top = get_sndedit_state_top_sample_number(state);
	int cur_top_channel = get_sndedit_state_top_channel_index(state);
	int channels = state->data->channels;
	int last_full_sample = cur_top + (total_rows - (channels - cur_top_channel))/channels;/*CHECK THIS*/
	
	if(channels >= total_rows){
		/*Make the given sample the top sample*/
		state->top_channel = 0;
		state->top.index = index;
		state->top.sample = node;
		state->row = 2;
	}
	else if(index > cur_top && index <= last_full_sample){
		/*Leave the screen where it is, just move rows*/
		int new_row = (index - cur_top) * channels - cur_top_channel;
		state->row = new_row + 2 /*Offset*/;
		
	}
	else if(index <= cur_top){
		/*Make the given sample the top sample*/
		state->top_channel = 0;
		state->top.index = index;
		state->top.sample = node;
		state->row = 2;
	}
	else if(index > last_full_sample){
			
		/*This becomes our last sample*/
		int new_top = index - (total_rows-channels) / channels - ((total_rows-channels) % channels == 0 ? 0 : 1);
		
		int remainder_rows = (total_rows-channels) % channels;
		int new_channel; /*HELP*/

		if(remainder_rows == 0){
			new_channel = 0;
		}
		else{
			new_channel = channels - (total_rows-channels) % channels;
		}
		
		for(; cur_top < new_top; cur_top++){
			state->top.sample = state->top.sample->next;
		}
		state->top.index = new_top;
		state->top_channel = new_channel;
		state->row = state->total_rows-channels;
	}
	return GOOD;
}

int move_sndedit_state_down(sndedit_state *state, int num){
	if(get_sndedit_state_can_move_down(state, num)){
		sndedit_node selected = get_sndedit_state_selected_sample(state);

		set_sndedit_selected_sample( state, selected.sample->next, selected.index + num);
	}
	else{
		/* Ignore command */
	}
}

int move_sndedit_state_up(sndedit_state *state, int num){
	if(get_sndedit_state_can_move_up(state, num)){
		sndedit_node selected = get_sndedit_state_selected_sample(state);
		sample_node *at_index = get_sample_by_index(state->data->sample_data_head, selected.index - num);

		set_sndedit_selected_sample( state, at_index, selected.index - num);
	}
	else{
		/* Ignore command */
	}
}

int get_sndedit_state_samples_per_page(sndedit_state *state){
	int num = state->total_rows - 2;
	num /= state->data->channels;
	return num;
}


sndedit_node get_mark_start(sndedit_state *state){
	sndedit_node selected = get_sndedit_state_selected_sample(state);
	int marked_index = state->marked->index;
	int selected_index = selected.index;

	int start_marked = MIN(marked_index, selected_index);
	int end_marked = MAX(marked_index, selected_index);

	sample_node *last;
	sample_node *first;
	if(start_marked == marked_index){
		last = selected.sample;
		first = state->marked->sample;
	}
	else{
		last = state->marked->sample;
		first = selected.sample;
	}

	sndedit_node result;
	result.index = start_marked;
	result.sample = first;
	return result;
}

sndedit_node get_mark_end(sndedit_state *state){
	sndedit_node selected = get_sndedit_state_selected_sample(state);
	int marked_index = state->marked->index;
	int selected_index = selected.index;

	int start_marked = MIN(marked_index, selected_index);
	int end_marked = MAX(marked_index, selected_index);

	sample_node *last;
	sample_node *first;
	if(start_marked == marked_index){
		last = selected.sample;
		first = state->marked->sample;
	}
	else{
		last = state->marked->sample;
		first = selected.sample;
	}

	sndedit_node result;
	result.index = end_marked;
	result.sample = last;
	return result;
}
int clone_node_except_next(sndedit_state *state, sample_node *node, sample_node **new_node){
	*new_node = create_sample_node(state->data->channels);
	if(*new_node){
		memcpy((*new_node)->channel_data, node->channel_data, sizeof(sound_reading) * state->data->channels);
		(*new_node)->next = NULL;
	}
	else{
		return UNABLE_TO_ALLOCATE_MEMORY;
	}
	return GOOD;
}

int convert_buffer_to_copy(sndedit_state *state, sample_buffer *buffer){
	sample_node *cur_old = buffer->start->next;
	sample_node *prev;
	int result;
	result = clone_node_except_next(state, buffer->start, &prev);
	return_if_not_GOOD(result);

	buffer->start = prev;
	if(buffer->size == 1) {
		buffer->end = prev;
		return GOOD;
	}
	sample_node *end = buffer->end->next;
	while(cur_old != end){
		sample_node *new_node;
		result = clone_node_except_next(state, cur_old, &new_node);
		return_if_not_GOOD(result);

		prev->next = new_node;

		cur_old = cur_old->next;
		prev = prev->next;
	}
	buffer->end = prev;
	return GOOD;
}

int copy_marked_into_buffer(sndedit_state *state){
	if(!state->marked){
		return INVALID_STATE;
	}

	if(state->buffer){
		sample_node *first = state->buffer->start;
		while(first){
			sample_node *next = first->next;
			free_sample_node(first);
			first = next;
		}
			
		free(state->buffer);
	}
	
	state->buffer = (sample_buffer*) malloc(sizeof(sample_buffer));
	if(!state->buffer){
		return UNABLE_TO_ALLOCATE_MEMORY;
	}

	sndedit_node start = get_mark_start(state);
	sndedit_node end = get_mark_end(state);

	state->buffer->size = end.index - start.index + 1;
	state->buffer->start = start.sample;
	state->buffer->end = end.sample;

	int result = convert_buffer_to_copy(state, state->buffer);
	return_if_not_GOOD(result);

	return GOOD;
}

int remove_all_marked(sndedit_state *state){
	sndedit_node start = get_mark_start(state);
	sndedit_node end = get_mark_end(state);

	if(start.index == 0){
		state->data->sample_data_head = end.sample->next;
	}
	else{
		sample_node *before_first = get_sample_by_index(state->data->sample_data_head, start.index - 1);
		before_first->next = end.sample->next;
	}

	sample_node *first = start.sample;
	int i;
	int num = get_sndedit_state_num_marked(state);
	for(i = 0; i < num; i++){
		sample_node *cur = first;
		first = first->next;
		free_sample_node(cur);
	}

	return GOOD;
}

int initiate_copy(sndedit_state *state){
	if(get_sndedit_state_allow_copy(state)){
		copy_marked_into_buffer(state);
		initiate_mark(state); /*Unmark the samples*/
	}
	else{
		/* Ignore command */
	}
	return GOOD;
}

int initiate_cut(sndedit_state *state){
	if(get_sndedit_state_allow_cut(state)){
		if(get_sndedit_state_num_marked(state) == state->data->samples){
			return GOOD;
		}
		copy_marked_into_buffer(state);
		int before_marked = get_mark_start(state).index - 1;
		remove_all_marked(state);
		set_sndedit_state_changed(state, true);
		if(before_marked < 0){
			set_sndedit_selected_sample(state, state->data->sample_data_head, 0);
		}
		else{
			set_sndedit_selected_sample(state, get_sample_by_index(state->data->sample_data_head, before_marked) , before_marked);
		}
		initiate_mark(state); /*Unmark the samples*/
	}
	else{
		/* Ignore command */
	}
	
}

int insert_after(sndedit_state *state, sample_node *node){
	sample_node *next = node->next;
	node->next = state->buffer->start;
	state->buffer->end->next = next;
	int result = convert_buffer_to_copy(state, state->buffer);

	return result;
}


int initiate_insert(sndedit_state *state, boolean after){
	if(get_sndedit_state_allow_insert(state)){
		if(after){
			insert_after(state, get_sndedit_state_selected_sample(state).sample);
			set_sndedit_state_changed(state, true);
		}
		else{
			insert_after(state, get_sample_by_index(state->data->sample_data_head, get_sndedit_state_selected_sample(state).index -1));
			set_sndedit_state_changed(state, true);
			int new_index = get_sndedit_state_selected_sample(state).index + get_sndedit_state_num_buffered(state);
			set_sndedit_selected_sample(state, get_sample_by_index(state->data->sample_data_head, new_index), new_index );
		}
	}
	else{
		/* Ignore command */
	}
	
}

int initiate_goto(sndedit_state *state){
	state->goto_mode = true;
	state->goto_index = 0;
	int i;
	for(i = 0; i < GOTO_STR_SIZE; i++){
		state->goto_str[i] = 0;
	}
}

int initiate_mark(sndedit_state *state){
	if(get_sndedit_state_allow_mark(state)){
		sndedit_node selected = get_sndedit_state_selected_sample(state);
		state->marked = (sndedit_node *) malloc(sizeof(sndedit_node));
		if(state->marked){
			state->marked->index = selected.index;
			state->marked->sample = selected.sample;
		}
		else{
			return UNABLE_TO_ALLOCATE_MEMORY;
		}
	}
	else if(get_sndedit_state_allow_unmark(state)){
		free(state->marked);
		state->marked = NULL;
	}
	else{
		return INVALID_STATE;
	}
	return GOOD;
}

int save_state(sndedit_state *state){
	if(get_sndedit_state_allow_save(state)){
		set_sndedit_state_changed(state, false);
		FILE *out = fopen(state->file_name, "w");
		if(out){
			int result = write_to_file_type(out, state->data, state->data->type);
			fclose(out);
			return result;
		}
		else{
			return COULDNT_OPEN_FILE;
		}
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

int should_be_marked(sndedit_state *state, int number){
	if(state->marked){
		int start = state->marked->index;
		int end = get_sndedit_state_selected_sample(state).index;
		if(number >= start && number <= end || number >= end && number <= start){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		return false;
	}
}

void initiate_end_goto(sndedit_state *state){
	state->goto_mode = false;
	int sample = atoi(state->goto_str);
	if(sample > 0 && sample < state->data->samples){
		set_sndedit_selected_sample(state, get_sample_by_index(state->data->sample_data_head, sample), sample);
	}
	else{

	}
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
	

	print_key_and_description_if_true(row, sidebar_start, "m", "mark   ", get_sndedit_state_allow_mark(state));
	print_key_and_description_if_true(row, sidebar_start, "m", "unmark ", get_sndedit_state_allow_unmark(state));
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
	//row++;
	mvprintw(row++, sidebar_start, "Row: %03d", state->row);

	output_num_chars('=', 20, row++, sidebar_start -1);	

	if(get_sndedit_state_show_marked(state))
		mvprintw(row++, sidebar_start, "   Marked: %d", get_sndedit_state_num_marked(state));
	else
		mvprintw(row++, sidebar_start, "                   ", get_sndedit_state_num_marked(state));

	if(get_sndedit_state_show_buffered(state))
		mvprintw(row++, sidebar_start, " Buffered: %d", get_sndedit_state_num_buffered(state));	
	else
		mvprintw(row++, sidebar_start, "                   ", get_sndedit_state_num_buffered(state));	

	free(sound_duration);
}

int print_samples(sndedit_state *state){
	sample_node *cur = get_sndedit_state_sample_at_top(state);
	int cur_channel_index = get_sndedit_state_top_channel_index(state);
	int sample_num = get_sndedit_state_top_sample_number(state);
	int i;
	for(i = 2; i < state->total_rows; i++){
		if(should_be_marked(state, sample_num)){
			attron(A_REVERSE);
		}
		else{
			attroff(A_REVERSE); 
		}
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
	attroff(A_REVERSE); 
}

int print_goto(sndedit_state *state){
	mvprintw(2, 1, "Goto sample: %s", state->goto_str);
}

int print_main_screen(sndedit_state *state){
	erase();
	print_centered_header(state);
	output_num_chars('=', state->total_cols, 1, 0);
	if(!state->goto_mode){
		print_sidebar(state);
		print_samples(state);
	}
	else{
		print_goto(state);
	}
}

int handle_input(sndedit_state *state, int in){
	int result;
	if(!state->goto_mode){	
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
			case 'g': 
				result = initiate_goto(state);
				break;
			case KEY_UP:
				result = move_sndedit_state_up(state, 1);
				break;
			case KEY_DOWN:
				result = move_sndedit_state_down(state, 1);
				break;
			case KEY_PPAGE:
				result = move_sndedit_state_up(state, get_sndedit_state_samples_per_page(state));
				break;
			case KEY_NPAGE:
				result = move_sndedit_state_down(state, get_sndedit_state_samples_per_page(state));
				break;
			default:
				break;
		}
	}
	else{
		switch(in){
		case KEY_ENTER:
		case '\n':
			initiate_end_goto(state);
			break;
		default:
			state->goto_str[state->goto_index++] = in;
			if(state->goto_index >= GOTO_STR_SIZE){
				initiate_end_goto(state);
			}
			break;
		}
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

	fclose(in);
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