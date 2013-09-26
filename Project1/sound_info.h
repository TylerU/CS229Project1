#ifndef GEN_HELPERS
	#include "gen_helpers.h"
	#define GEN_HELPERS
#endif

sound_file *create_empty_sound_file_data();

sample_node *create_sample_node(int channels);

file_type get_file_type(FILE *in);

int get_sound_info(FILE* in, sound_file *data);

int read_aiff_file(FILE* in, sound_file *data);

int read_cs229_file(FILE* in, sound_file *data);

char* file_type_to_string(file_type type);

char *get_sound_duration_string(sound_file *data);

int consume_whitespace(FILE *in);

int is_end_of_line(FILE *in, int c);

int consume_until_endline(FILE *in);

int get_until_whitespace_or_max_len(FILE *in, char buffer[], int max);

int store_key_val_pair(sound_file *data, char *identifier, int val);

int read_header_key_value_pair(FILE *in, sound_file *data);

int read_cs229_header(FILE* in, sound_file *data);

int read_samples_data(FILE *in, sound_file *data);

int check_for_valid_header_data(sound_file *data);

int get_unsigned_four_byte_int(FILE *in, unsigned int *place);

int get_four_byte_string(FILE *in, char *storage);

int get_int_from_memory(char *mem, void *dest, int size);

int process_comm_chunk(char* chunk, sound_file *data);

int get_next_sample_from_aiff(sound_file *data, char *chunk, sample_node **new_node_return);

int process_ssnd_chunk(char *chunk, sound_file *data, int size);

int read_aiff_chunk(char id[5], char* chunk, int chunk_size, sound_file *data);

int attempt_read_aiff_chunk(FILE *in, sound_file *data, unsigned int* bytes_remaining);

int read_aiff_chunks(FILE *in, sound_file *data, unsigned int *bytes_remaining);