#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

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

const char* error_descriptions[] = {"UNEXPECTED_EOF or Input Error","OK","UNRECOGNIZED_FILE_FORMAT","WRONG_NUMBER_OF_SOUND_READINGS","NOT_ENOUGH_SAMPLES",
	"Empty sample data line or not enough valid channels data", "Invalid or unspecified bit depth", "Invalid or unspecified channels value", "Invalid or unspecified sample rate",
	"Encountered invalid header identifier or beginning of sample data without proper specifier", "Not an error, encountered the beginning of start data",
	"Invalid header value or unexpected end of file", "Unable to allocate more memory"};


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
	sample_rate sample_rate = data->sample_rate;
	int total_seconds;
	if(sample_rate == 0){
		strcpy(result, "ERROR");
		return;
	}
	total_seconds = samples / sample_rate;
	sprintf(result, "%01d:%02d:%05.2Lf", total_seconds/3600, (total_seconds%3600)/60, samples/sample_rate - (total_seconds/60)*60);

	return result;
}

int find_string_and_get_following(FILE* in, char *find, char *last){
	int c;
	while((c = fgetc(in)) != EOF && c == *(find) && *find != '\0'){
		find++;
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
	char *dest = (char*)place;
	int result = fread(dest, size, 1, in);
	flip_endian(dest, size);
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
	sample_rate sample_rate = 0;
	get_int_from_memory(chunk, &num_channels, 2);
	chunk+=2;
	get_int_from_memory(chunk, &num_samples, 4);
	chunk+=4;
	get_int_from_memory(chunk, &sample_size, 2);
	chunk+=2;

	memcpy(&sample_rate, chunk, SAMPLE_RATE_SIZE);
	chunk+=10;
	flip_endian((char*)&sample_rate, SAMPLE_RATE_SIZE);
	
	data->bit_depth=sample_size;
	data->channels=num_channels;
	data->samples=num_samples;
	data->sample_rate=sample_rate;
	return OK;
}

int get_next_sample_from_aiff(sound_file *data, char *chunk, sample_node **new_node_return){
	sample_node *node = create_sample_node(data->channels);
	sound_reading *cur_sample_value;
	int i;

	if(!node)
		return UNABLE_TO_ALLOCATE_MEMORY;

	cur_sample_value = node->channel_data;

	for(i = 0; i < data->channels; i++){
		switch(data->bit_depth){
		case 8:
			{
				char result;
				get_int_from_memory(chunk, &result, 1);
				chunk+=1;
				*cur_sample_value = (sound_reading) result;
			}
			break;
		case 16:
			{
				short result;
				get_int_from_memory(chunk, &result, 2);
				chunk+=2;
				*cur_sample_value = (sound_reading) result;
			}
			break;
		case 32:
			{
				int result;
				get_int_from_memory(chunk, &result, 4);
				chunk+=4;
				*cur_sample_value = (sound_reading) result;
			}
			break;
		default:
			return INVALID_BIT_DEPTH;
			break;
		}
	}

	*new_node_return = node;
	return OK;
}

int process_ssnd_chunk(char *chunk, sound_file *data, int size){
	unsigned int offset = 0;
	unsigned int block_size = 0;
	unsigned int min_block_size = data->channels * data->bit_depth/8;
	unsigned int actual_block_size = min_block_size;
	int remaining_size = size;
	sample_node *last_node = NULL;

	get_int_from_memory(chunk, &offset, 4);
	chunk += 4;/*bad system here...just don't ever foreget to increment..ever*/
	get_int_from_memory(chunk, &block_size, 4);
	chunk += 4;
	chunk += offset;
	remaining_size -= 8 + offset;/*Dont forget to update me if this section changes*/

	if(block_size != 0 && actual_block_size % block_size != 0){
		actual_block_size += block_size - actual_block_size % block_size;
	}

	/*at the beginning of the sample data*/
	while(remaining_size > actual_block_size){
		sample_node *next_node = NULL;
		get_next_sample_from_aiff(data, chunk, &next_node);
		if(last_node != NULL){
			last_node->next = next_node;
		}
		else{
			data->sample_data_head = next_node;
		}
		last_node = next_node;
		chunk+=actual_block_size;
		remaining_size-=actual_block_size;
	}
}

int read_aiff_chunk(char id[5], char* chunk, int chunk_size, sound_file *data){
	if(strcmp(id, "COMM")==0){
		process_comm_chunk(chunk, data);
	}
	else if(strcmp(id, "SSND") == 0){
		process_ssnd_chunk(chunk, data, chunk_size);
	}
	else{
	/*do nothing*/
	}
}

int attempt_read_aiff_chunk(FILE *in, sound_file *data, unsigned int* bytes_remaining){
	char id[5];
	unsigned int chunk_size = 0;
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

int write_229_pre_header(FILE *out){
	fprintf(out, "CS229\n\n# This file generated by sound file conversion utility sndconv\n");
}

int write_229_header(FILE *out, sound_file *data){
	fprintf(out, "SampleRate %d\n", data->sample_rate);
	fprintf(out, "Samples    %d\n", data->samples);
	fprintf(out, "Channels   %d\n", data->channels);
	fprintf(out, "BitDepth   %d\n", data->bit_depth);
	fprintf(out, "StartData\n");
}

int write_229_sound_data(FILE *out, sound_file *data){
	sample_node *cur = data->sample_data_head;
	while(cur){
		int i;
		for(i = 0; i < data->channels; i++){
			fprintf(out, "%d ", cur->channel_data + i);
		}
		printf("\n");
		cur=cur->next;
	}
}

int write_aiff_pre_header(FILE *out){
	fprintf(out, "FORM");
}


unsigned int get_aiff_comm_chunk_size(sound_file *data){
	return 26;
}

unsigned int get_aiff_ssnd_chunk_size(sound_file *data){
	return 16 + data->samples * data->bit_depth/8;
}


unsigned int get_aiff_file_size(sound_file *data){
	unsigned int result = 0;
	result += 4;
	result += 4;
	result += 4;
	result += get_aiff_comm_chunk_size(data);
	result += get_aiff_ssnd_chunk_size(data);

	return result;
}

int write_number_to_aiff(void *what, size_t size, FILE *out){
	void *temp = malloc(size);
	if(!temp) return UNABLE_TO_ALLOCATE_MEMORY;
	
	flip_endian((char*)temp, size);
	fwrite(temp, size, 1, out);

	free(temp);
	return OK;
}

int write_aiff_file_size(FILE *out, sound_file *data){
	unsigned int bytes_remaining = get_aiff_file_size(data) - 8;
	write_number_to_aiff(&bytes_remaining, sizeof(bytes_remaining), out);
	/*CHECK FOR ERROR*/
}

int write_aiff_post_header(FILE *out){
	fprintf(out, "AIFF");
}

int write_comm_chunk(FILE *out, sound_file *data){
	int remaining_comm_size = get_aiff_comm_chunk_size(data) - 8;
	short int channels = data->channels;
	unsigned int total_samples = data->samples;
	short int sample_size = data->bit_depth;
	sample_rate sample_rate = data->sample_rate;

	fprintf(out, "COMM");
	write_number_to_aiff(&remaining_comm_size, sizeof(remaining_comm_size), out);
	write_number_to_aiff(&channels, sizeof(channels), out);
	write_number_to_aiff(&total_samples, sizeof(total_samples), out);
	write_number_to_aiff(&sample_size, sizeof(sample_size), out);
	write_number_to_aiff(&sample_rate, sizeof(sample_rate), out);
}

int write_single_aiff_sample(sound_reading *what, sound_file *data, FILE *out){
	switch(data->bit_depth){
	case 8:
		{
			char result = (char) (*what);
			write_number_to_aiff(&result, sizeof(result), out);
		}
		break;
	case 16:
		{
			short result = (short) (*what);
			write_number_to_aiff(&result, sizeof(result), out);
		}
		break;
	case 32:
		{
			int result = (int) (*what);
			write_number_to_aiff(&result, sizeof(result), out);
		}
		break;
	default:
		break;
	}
}

int write_ssnd_chunk(FILE *out, sound_file *data){
	int remaining_ssnd_size = get_aiff_ssnd_chunk_size(data) - 8;
	unsigned int zero = 0;
	sample_node *cur = data->sample_data_head;

	fprintf(out, "SSND");
	write_number_to_aiff(&remaining_ssnd_size, sizeof(remaining_ssnd_size), out);
	write_number_to_aiff(&zero, sizeof(zero), out);/*Offset*/
	write_number_to_aiff(&zero, sizeof(zero), out);/*Block size*/

	
	while(cur){
		int i;
		for(i = 0; i < data->channels; i++){
			write_single_aiff_sample(cur->channel_data + i, data, out);
		}
		cur=cur->next;
	}
}


int write_to_aiff(FILE* out, sound_file *data){
	write_aiff_pre_header(out);
	write_aiff_file_size(out, data);
	write_aiff_post_header(out);
	write_comm_chunk(out, data);
	write_ssnd_chunk(out, data);
}

int write_to_cs229(FILE *out, sound_file *data){
	write_229_pre_header(out);
	write_229_header(out, data);
	write_229_sound_data(out, data);
}

int sound_conv(){
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
		fclose(in);
	}
	else{
	}

	if(result == OK){
		if(file_data->type == CS229){
			FILE *outf = fopen( strcat(file_name, ".aiff") , "w");
			write_to_aiff(outf, file_data);
		}
		else if (file_data->type == AIFF){
			FILE *outf = fopen( strcat(file_name, ".cs229") , "w");
			write_to_cs229(outf, file_data);
		}
		else{

		}
	}
	else{
		printf("Error Code %d: %s\n", result, error_descriptions[result]);
	}

}

void format_output(sound_file *file_data, char* file_name){
	printf("------------------------------------------------------------\n");
	printf("Filename: %s\n", file_name);
	printf("Format: %s\n", file_type_to_string(file_data->type));
	printf("Sample Rate: %0Lg\n", file_data->sample_rate);
	printf("Bit Depth: %d\n", file_data->bit_depth);
	printf("Channels: %d\n", file_data->channels);
	printf("Samples: %d\n", file_data->samples);
	printf("Duration: %s\n", get_sound_duration_string(file_data));
	printf("------------------------------------------------------------\n");
	
	if(DEBUG){
		sample_node *cur = file_data->sample_data_head;
		//while(cur){
			int i;
			for(i = 0; i < file_data->channels; i++){
				printf("%hd ", cur->channel_data + i);
			}
			printf("\n");
			cur=cur->next;
		//}
	}
}

int main(int argc, char* argv[]){
	int result = 0;
	FILE *in;
	char file_name[DEFAULT_BUFFER_LENGTH] = "Not Implemented";
	sound_file *file_data = create_empty_sound_file_data(); 
	if(DEBUG){
		strcpy(file_name, "hello.aiff");
	}
	else{
		printf("Enter the pathname of a sound file:\n");
		fgets(file_name, DEFAULT_BUFFER_LENGTH, stdin);
		file_name[strlen(file_name)-1]=0;
	}

	in = fopen(file_name, "rb");
	if(in){
		result = get_sound_info(in, file_data);
		fclose(in);
	}
	else{
		printf("Unable to open file\n");
	}

	if(result == OK){
		format_output(file_data, file_name);
	}
	else{
		printf("Error Code %d: %s\n", result, error_descriptions[result]);
	}


	if(WINDOWS){
		system("pause");
	}

	return result;
}
