#define DEBUG 0
#define WINDOWS 1

#define DEFAULT_BUFFER_LENGTH 150
#define DEFAULT_SAMPLES -1
#define DEFAULT_BIT_DEPTH -1
#define DEFAULT_CHANNELS -1
#define DEFAULT_SAMPLE_RATE -1
#define SAMPLE_RATE_SIZE 8 

#define return_if_not_OK(X) if(X!=OK) return X;
#define return_if_falsey(X) if(!X) return X;

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
} error_code;

const char* error_descriptions[];


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