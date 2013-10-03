#ifndef GEN_HELPERS_H
#define GEN_HELPERS_H

#define DEBUG 0
#define WINDOWS 0

#define DEFAULT_BUFFER_LENGTH 150
#define DEFAULT_SAMPLES -1
#define DEFAULT_BIT_DEPTH -1
#define DEFAULT_CHANNELS -1
#define DEFAULT_SAMPLE_RATE -1
#define SAMPLE_RATE_SIZE 10

#define return_if_not_OK(X) if(X!=OK) return X;
#define return_if_falsey(X) if(!X) return X;

#define SNDINFO_README_FILE "sndinfo_readme.txt"
#define SNDCONV_README_FILE "sndconv_readme.txt"
#define README_HEADER_FILE "readme_header.txt"
#define README_FILE "README"

typedef enum {
	UNEXPECTED_EOF,
	OK,
	UNRECOGNIZED_FILE_FORMAT,
	WRONG_NUMBER_OF_SOUND_READINGS,
	NOT_ENOUGH_SAMPLES,
	NOT_ENOUGH_CHANNELS_IN_SAMPLE,
	INVALID_BIT_DEPTH,
	INVALID_CHANNELS_VALUE,
	INVALID_SAMPLE_RATE,
	INVALID_HEADER_IDENTIFIER,
	ENCOUNTERED_START_DATA,
	INVALID_HEADER_VALUE,
	UNABLE_TO_ALLOCATE_MEMORY,
	COULDNT_OPEN_FILE
} error_code;

const char* error_descriptions[14];


typedef enum {
	CS229,
	AIFF,
	UNRECOGNIZED
} file_type;

typedef int sound_reading;
typedef long double sample_rate;

typedef struct sample_node {
	sound_reading *channel_data;
	struct sample_node *next;
} sample_node;

typedef struct {
	int channels;
	int samples;
	int bit_depth;
	sample_rate sample_rate;
	file_type type;
	sample_node *sample_data_head;
} sound_file;

int find_string_and_get_following(FILE* in, char *find, char *last);

int find_string_and_ensure_following_whitespace(FILE *in, char *find);

int flip_endian(char *dest, int size);

int print_readme(char file_name[], FILE* out);

void print_if_error(int err_code, char file_name[]);

#endif