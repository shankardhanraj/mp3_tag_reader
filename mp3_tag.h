#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#ifdef MP3_TAG_H
//#define MP3_TAG_H
#define START_OF_HEADER 0
#define END_OF_HEADER 10
#define SIZE_OF_HEADER 10

typedef enum {TALB, TIT2, TYER, TRCK, TPE1, COMM, APIC} FRAME_NAME; 
typedef enum {e_success, e_failure} status;

typedef struct 
{
	unsigned char id[4];
	unsigned char ver;
	unsigned char rev;
	unsigned char flags;
	unsigned int  tag_size;
	unsigned long int file_size;
	char *tag_data;
} tag_header;

typedef struct
{
	unsigned char name[5];
	unsigned char status_flag;
	unsigned char encoding_flag;
	unsigned char encoding_type;
	unsigned int  frame_data_size;
	unsigned int  name_offset;
	unsigned int  frame_size_offset;
	unsigned int  status_flag_offset;
	unsigned int  encoding_flag_offset;
	unsigned int  encoding_type_offset;
	unsigned int  frame_data_start_offset;
	unsigned int  frame_data_end_offset;
	char 		  *frame_data;
} tag_frames;
//#endif
