#ifndef GEN_HELPERS
	#include "gen_helpers.h"
	#define GEN_HELPERS
#endif

int write_229_pre_header(FILE *out);

int write_229_header(FILE *out, sound_file *data);

int write_229_sound_data(FILE *out, sound_file *data);

int write_aiff_pre_header(FILE *out);

unsigned int get_aiff_comm_chunk_size(sound_file *data);

unsigned int get_aiff_ssnd_chunk_size(sound_file *data);

unsigned int get_aiff_file_size(sound_file *data);

int write_number_to_aiff(void *what, size_t size, FILE *out);

int write_aiff_file_size(FILE *out, sound_file *data);

int write_aiff_post_header(FILE *out);

int write_comm_chunk(FILE *out, sound_file *data);

int write_single_aiff_sample(sound_reading *what, sound_file *data, FILE *out);

int write_ssnd_chunk(FILE *out, sound_file *data);

int write_to_aiff(FILE* out, sound_file *data);

int write_to_cs229(FILE *out, sound_file *data);