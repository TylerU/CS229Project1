#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define DEBUG 1
#define DEFAULT_BUFFER_LENGTH 150
#define DEFAULT_SAMPLES -1
#define DEFAULT_BIT_DEPTH -1
#define DEFAULT_CHANNELS -1
#define DEFAULT_SAMPLE_RATE -1

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
} error_code;

const char* error_descriptions[] = {"UNEXPECTED_EOF or Input Error","OK","UNRECOGNIZED_FILE_FORMAT","WRONG_NUMBER_OF_SOUND_READINGS","NOT_ENOUGH_SAMPLES",
	"Empty sample data line or not enough valid channels data", "Invalid or unspecified bit depth", "Invalid or unspecified channels value", "Invalid or unspecified sample rate",
	"Encountered invalid header identifier or beginning of sample data without proper specifier", "Not an error, encountered the beginning of start data",
	"Invalid header value or unexpected end of file"};


typedef enum {
	CS229,
	AIFF,
	UNRECOGNIZED
} file_type;

typedef int sound_reading;


typedef struct sample_node {
	sound_reading *channel_data;
	struct sample_node *next;
} sample_node;

typedef struct {
	int channels;
	int samples;
	int bit_depth;
	int sample_rate;
	file_type type;
	sample_node *sample_data_head;
} sound_file;


sound_file *create_empty_sound_file_data(){
	sound_file *sound_data = (sound_file *)malloc(sizeof(sound_file));
	if(sound_data){
		sound_data->samples = DEFAULT_SAMPLES;
		sound_data->bit_depth = DEFAULT_BIT_DEPTH;
		sound_data->channels = DEFAULT_CHANNELS;
		sound_data->sample_rate = DEFAULT_SAMPLE_RATE;
		sound_data->type = UNRECOGNIZED;
		return sound_data;
	}
	else{
		return NULL;
	}
}

sample_node *create_sample_node(int channels){
	sample_node *node = (sample_node*)malloc(sizeof(sample_node));
	if(node){
		node->next = NULL;
		node->channel_data = (sound_reading*)malloc(sizeof(sound_reading) * channels);
		if(node->channel_data){
			return node;
		}
		else{
			return NULL;
		}
	}
	else{
		return NULL;
	}
}

char* file_type_to_string(file_type type){
	switch(type){
	case(AIFF):
		return "AIFF";
		break;
	case(CS229):
		return "CS229";
		break;
	default:
		return "Unrecognized File Type";
		break;
	}
}

/*PRE: Buffer must be at very beginning of the file*/
file_type get_file_type(FILE *in){
	int first_char; 
	first_char = fgetc(in);
	ungetc(first_char, in);
	if(first_char == 'F'){
		return AIFF;
	}
	else if (first_char == 'C'){
		return CS229;
	}
	else{
		return UNRECOGNIZED;
	}
}

char *get_sound_duration_string(sound_file *data){
	char *result = (char*)malloc(sizeof(char) * 20);
	int samples = data->samples;
	int sample_rate = data->sample_rate;
	int total_seconds = samples / sample_rate;
	sprintf(result, "%01d:%0d:%05.2f", total_seconds/3600, (total_seconds%3600)/60, 1.0*samples/sample_rate-samples/sample_rate);

	return result;
}

int find_string_and_get_following(FILE* in, char *find, char *last){
	int c;
	while((c = fgetc(in)) != EOF && c == *(find++) && c != '\0'){
	}
	ungetc(c, in);
	*last = c;
	if((*find)=='\0'){
		return OK;
	}
	else if (c==EOF){
		return UNEXPECTED_EOF;
	}
	else {
		return 0;
	}
}

int find_string_and_ensure_following_whitespace(FILE *in, char *find){
	char last_char;
	int result;
	if((result = find_string_and_get_following(in, find, &last_char)) && isspace(last_char)){
		return OK;
	}
	else{
		return result;
	}
}

int consume_whitespace(FILE *in){
	int c;
	while(isspace(c = fgetc(in))){
	}
	ungetc(c, in);
	return OK;
}


int is_end_of_line(FILE *in, int c)
{
    int eol = (c == '\r' || c == '\n');
    if (c == '\r')
    {
        c = getc(in);
        if (c != '\n' && c != EOF)
            ungetc(c, in);
    }
    return eol;
}

int consume_until_endline(FILE *in){
	int c;

	for(c = fgetc(in); c != EOF && !is_end_of_line(in, c); c = fgetc(in)){
	}
	return OK;
}

int get_until_whitespace_or_max_len(FILE *in, char buffer[], int max){
	int c;
	int cur_pos = 0;
	for(c = fgetc(in); c != EOF && !isspace(c) && cur_pos < max-1; c=fgetc(in), cur_pos++){
		buffer[cur_pos] = c;
	}
	if(isspace(c)){/*in case the caller is looking for whitespace later*/
		ungetc(c, in);
	}
	buffer[cur_pos] = '\0';
	return OK;
}

int store_key_val_pair(sound_file *data, char *identifier, int val){
	if(strcmp(identifier, "SampleRate") == 0){
		data->sample_rate = val;
	}
	else if(strcmp(identifier, "Samples") == 0){
		data->samples = val;
	}
	else if(strcmp(identifier, "Channels") == 0){
		data->channels = val;
	}
	else if(strcmp(identifier, "BitDepth") == 0){
		data->bit_depth = val;
	}
	else{/*error*/
		return INVALID_HEADER_IDENTIFIER;
	}
	return OK;
}

int read_header_key_value_pair(FILE *in, sound_file *data){
	char *valid_keywords[5] = {"SampleRate","Samples","Channels","BitDepth", "StartData"};
	char read_identifier[DEFAULT_BUFFER_LENGTH];
	int value;
	int result;

	get_until_whitespace_or_max_len(in, read_identifier, DEFAULT_BUFFER_LENGTH);/*What if this hits max length????!?!??!!*/
	if(strcmp(read_identifier, "StartData") == 0){
		consume_until_endline(in);
		return ENCOUNTERED_START_DATA;
	}
	else{
		result = fscanf(in, "%d", &value);
		if(result != 1)
			return INVALID_HEADER_VALUE;

		consume_until_endline(in);

		result = store_key_val_pair(data, read_identifier, value);
		return result;
	}
}
/*Assumes buffer is at beginning of a the first line of the file after the CS229 line*/
int read_cs229_header(FILE* in, sound_file *data){
	while(1){
		int result;
		int start_char = fgetc(in);
		ungetc(start_char, in);
		
		if(start_char == '#'){
			consume_until_endline(in);
		}
		else if(isspace(start_char)){
			consume_until_endline(in);
		}
		else{
			int result = read_header_key_value_pair(in, data);
			if(result == ENCOUNTERED_START_DATA)
				break;
			else if(result == INVALID_HEADER_IDENTIFIER || result == INVALID_HEADER_VALUE)
				return result;
		}
	}
	return OK;
}

int read_samples_data(FILE *in, sound_file *data){
	sample_node *cur_sample_node, *last_sample_node;
	int num_samples = 0;
	cur_sample_node = NULL;
	last_sample_node = NULL;
	
	if(data->samples > 0 || data->samples == DEFAULT_SAMPLES){
		while(data->samples != DEFAULT_SAMPLES && num_samples < data->samples || data->samples==DEFAULT_SAMPLES){/*in the case of DEFAULT_SAMPLES, we will break out of this loop*/
			sound_reading *cur_sample;
			int i;
			cur_sample_node = create_sample_node(data->channels);
			if(last_sample_node)
				last_sample_node->next = cur_sample_node;
			else
				data->sample_data_head = cur_sample_node;

			cur_sample = (cur_sample_node->channel_data);


			for(i = 0; i < data->channels; i++){
				sound_reading value = 0;
				int cur_char;
				int scanf_result;
				for(cur_char = fgetc(in); isspace(cur_char); cur_char = fgetc(in)){
					if(is_end_of_line(in, cur_char)){
						return NOT_ENOUGH_CHANNELS_IN_SAMPLE;
					}
				}
				ungetc(cur_char, in);
				scanf_result = fscanf(in, "%d", &value);/*figure out what type this should be*/
				if(scanf_result == 1){
					*(cur_sample+i) = value;
				}
				else{
					break;
				}
			}
			consume_until_endline(in);
			if(i < data->channels){
				if(i==0){/*This is the last input*/
					break;
				}
				else{/*Error - We can't get the rest of the channels for this guy*/
					return WRONG_NUMBER_OF_SOUND_READINGS;
				}
			}
			else{
				/*Everything was read correctly*/
			}
			num_samples++;
			last_sample_node = cur_sample_node;
		}
		if(data->samples != DEFAULT_SAMPLES && data->samples != num_samples){
			return NOT_ENOUGH_SAMPLES;
		}

		if(data->samples == DEFAULT_SAMPLES){
			data->samples = num_samples;
		}
	}
	return OK;
}

int check_for_valid_header_data(sound_file *data){
	if(data->bit_depth != 8 && data->bit_depth != 16 && data->bit_depth != 32)
		return INVALID_BIT_DEPTH;

	if(data->channels == DEFAULT_CHANNELS)
		return INVALID_CHANNELS_VALUE;

	if(data->sample_rate == DEFAULT_SAMPLE_RATE)
		return INVALID_SAMPLE_RATE;

	if(data->type == UNRECOGNIZED)
		return UNRECOGNIZED_FILE_FORMAT;

	return OK;
}

int read_cs229_file(FILE* in, sound_file *data){
	int result;
	result = find_string_and_ensure_following_whitespace(in, "CS229");
	return_if_not_OK(result);
	result = consume_until_endline(in);
	return_if_not_OK(result);
	result = read_cs229_header(in, data);
	return_if_not_OK(result);
	result = check_for_valid_header_data(data);
	return_if_not_OK(result);
	result = read_samples_data(in, data);
	return_if_not_OK(result);
	return OK;
}

int get_unsigned_four_byte_int(FILE *in, unsigned int *place){
	int size = 4;
	char *temp = (char*)malloc(size);
	char *dest = (char*)place;
	int result = fread(temp, size, 1, in);
	int i;
	for(i = 0; i < size; i++){
		*(dest + i) = *(temp+size-1-i);
	}
	free(temp);
	return result == 1 ? OK : UNEXPECTED_EOF;
}

int get_four_byte_string(FILE *in, char *storage){
	int result = fread(storage, 1, 4, in);
	if(result == 4){
		*(storage + 4) = '\0';

		return OK;
	}
	else{
		return UNEXPECTED_EOF;
	}
}

int get_int_from_memory(char *mem, void *dest, int size){
	memcpy(dest, mem, size);
	flip_endian((char*)dest, size);
}
int flip_endian(char *dest, int size){
	char* temp = (char*)malloc(size);
	int i;
	memcpy(temp, dest, size);
	for(i = 0; i < size; i++){
		*(dest + i) = *(temp+size-1-i);
	}

	free(temp);
}
int process_comm_chunk(char* chunk, sound_file *data){
	short int num_channels;
	unsigned int num_samples;
	short int sample_size;
	long double sample_rate;
	get_int_from_memory(chunk, &num_channels, 2);
	chunk+=2;
	get_int_from_memory(chunk, &num_samples, 4);
	chunk+=4;
	get_int_from_memory(chunk, &sample_size, 2);
	chunk+=2;
	memcpy(&sample_rate, chunk, 8);
	chunk+=10;
	data->bit_depth=sample_size;
	data->channels=num_channels;
	data->samples=num_samples;
	data->sample_rate=sample_rate;

	//printf("%d\n", sizeof(long double));
	//printf("Channels: %hu\n", num_channels);
	//printf("Samples: %u\n", num_samples);
	//printf("Sample Size: %hi\n", sample_size);
	//printf("Sample Rate: %lf\n", sample_rate);
}

int process_ssnd_chunk(char *chunk, sound_file *data){

}

int read_aiff_chunk(char id[5], char* chunk, int chunk_size, sound_file *data){
	if(strcmp(id, "COMM")==0){
		process_comm_chunk(chunk, data);
	}
	else if(strcmp(id, "SSND")){
		process_ssnd_chunk(chunk, data);
	}
	else{
	/*do nothing*/
	}
}

int attempt_read_aiff_chunk(FILE *in, sound_file *data, unsigned int* bytes_remaining){
	char id[5];
	unsigned int chunk_size;
	unsigned int total_chunk_size;
	char *temp;
	int content_block_size;
	int result = get_four_byte_string(in, id);
	return_if_not_OK(result);
	result = get_unsigned_four_byte_int(in, &chunk_size);
	return_if_not_OK(result);

	total_chunk_size = chunk_size + 8;
	if(total_chunk_size % 2 == 1){/*odd*/
		total_chunk_size+=1;
	}
	
	content_block_size = total_chunk_size - 8;
	temp = (char*)malloc(content_block_size);
	fread(temp, 1, content_block_size, in);
	read_aiff_chunk(id, temp, chunk_size, data);
	free(temp);
	*(bytes_remaining) -= total_chunk_size;
	return OK;
}

int read_aiff_chunks(FILE *in, sound_file *data, unsigned int *bytes_remaining){
	while(*bytes_remaining > 0){/*Super high chance this fails*/
		int result = attempt_read_aiff_chunk(in, data, bytes_remaining);
		return_if_not_OK(result);
	}
	return OK;
}

int read_aiff_file(FILE* in, sound_file *data){
	int result;
	unsigned int bytes_remaining;
	char temp;
	result = find_string_and_get_following(in, "FORM", &temp);
	return_if_not_OK(result);
	result = get_unsigned_four_byte_int(in, &bytes_remaining);
	return_if_not_OK(result);
	result = find_string_and_get_following(in, "AIFF", &temp);
	bytes_remaining-=4;
	return_if_not_OK(result);
	result = read_aiff_chunks(in, data, &bytes_remaining);
	return_if_not_OK(result);
	return OK;
}

int get_sound_info(FILE* in, sound_file *data){
	file_type type = get_file_type(in);
	 
	if(type != UNRECOGNIZED){
		data->type = type;
		if(type == AIFF){
			read_aiff_file(in, data);
		}
		else if (type == CS229){
			read_cs229_file(in, data);
		}
		else{
			return UNRECOGNIZED_FILE_FORMAT;
		}
	}
	else{
		return UNRECOGNIZED_FILE_FORMAT;
	}
}

void format_output(sound_file *file_data, char* file_name){
	printf("------------------------------------------------------------\n");
	printf("Filename: %s\n", file_name);
	printf("Format: %s\n", file_type_to_string(file_data->type));
	printf("Sample Rate: %d\n", file_data->sample_rate);
	printf("Bit Depth: %d\n", file_data->bit_depth);
	printf("Channels: %d\n", file_data->channels);
	printf("Samples: %d\n", file_data->samples);
	printf("Duration: %s\n", get_sound_duration_string(file_data));
	printf("------------------------------------------------------------\n");
}

int main(int argc, char* argv[]){
	int result = 0;
	FILE *in;
	char file_name[DEFAULT_BUFFER_LENGTH] = "Not Implemented";
	sound_file *file_data = create_empty_sound_file_data(); 
	if(DEBUG){
		strcpy(file_name, "zep.aiff");
	}

	in = fopen(file_name, "rb");
	if(in){
		result = get_sound_info(in, file_data);
	}
	else{
	}
	fclose(in);

	if(result == OK){
		format_output(file_data, file_name);
	}
	else{
		printf("Error Code %d: %s\n", result, error_descriptions[result]);
	}


	if(DEBUG){
		system("pause");
	}
	return result;
}