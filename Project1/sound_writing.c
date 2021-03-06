#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gen_helpers.h"
#include "sound_writing.h"

int write_229_pre_header(FILE *out){
	fprintf(out, "CS229\n\n# This file generated by sound file conversion utility sndconv\n");
	return GOOD;
}

int write_229_header(FILE *out, sound_file *data){
	fprintf(out, "SampleRate %0.0Lf\n", data->sample_rate);
	fprintf(out, "Samples    %d\n", data->samples);
	fprintf(out, "Channels   %d\n", data->channels);
	fprintf(out, "BitDepth   %d\n", data->bit_depth);
	fprintf(out, "StartData\n");
	return GOOD;
}

int write_229_sound_data(FILE *out, sound_file *data){
	sample_node *cur = data->sample_data_head;
	while(cur){
		int i;
		for(i = 0; i < data->channels; i++){
			fprintf(out, "%d ", *(cur->channel_data + i));
		}
		fprintf(out, "\n");
		cur=cur->next;
	}
	return GOOD;
}

int write_aiff_pre_header(FILE *out){
	fprintf(out, "FORM");
	return GOOD;
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
	memcpy(temp, what, size);
	flip_endian((char*)temp, size);
	fwrite(temp, size, 1, out);

	free(temp);
	return GOOD;
}

int write_aiff_file_size(FILE *out, sound_file *data){
	unsigned int bytes_remaining = get_aiff_file_size(data) - 8;
	write_number_to_aiff(&bytes_remaining, sizeof(bytes_remaining), out);
	/*CHECK FOR ERROR*/
	return GOOD;
}

int write_aiff_post_header(FILE *out){
	fprintf(out, "AIFF");
	return GOOD;
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
	write_number_to_aiff(&sample_rate, SAMPLE_RATE_SIZE, out);
	return GOOD;
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
	return GOOD;
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
	return GOOD;
}


int write_to_aiff(FILE* out, sound_file *data){
	write_aiff_pre_header(out);
	write_aiff_file_size(out, data);
	write_aiff_post_header(out);
	write_comm_chunk(out, data);
	write_ssnd_chunk(out, data);
	return GOOD;
}

int write_to_cs229(FILE *out, sound_file *data){
	write_229_pre_header(out);
	write_229_header(out, data);
	write_229_sound_data(out, data);
	return GOOD;
}

int write_to_file_type(FILE* out, sound_file *file_data, file_type type){
	if(type == AIFF){
		return write_to_aiff(out, file_data);
	}
	else if(type == CS229){
		return write_to_cs229(out, file_data);
	}
	else{
		return UNRECOGNIZED_FILE_FORMAT;
	}
}

