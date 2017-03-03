#include "mp3_tag.h"

/* Function prototypes */
unsigned int convert_bigEndian_to_littleEndian_4Byte(unsigned int num);
unsigned int convert_littleEndian_to_bigEndian_4Byte(unsigned int little_endian);
unsigned int convert_32BitInteger_to_syncSafeInteger(unsigned int integer_32_bit);
unsigned int convert_syncSafeInteger_to_32BitInteger(unsigned int sync_safe_integer);
unsigned int search_frame(char *tag_data, char *frame_name, unsigned int tag_size);
unsigned int mem_alloc(char *tag_data, unsigned int tag_size, unsigned int size);
status find_image(char *tag_data, unsigned int tag_size);

status edit_frame_data(FILE *fs, FRAME_NAME frame_name, char *new_data);
status create_frame(FILE * fs, tag_frames ** frame_t, char *tag_data, int tag_size, FRAME_NAME frameName, char *new_data);
status my_exit(char **cmd_args);

extern tag_header header;
extern tag_frames *frame_t[6];
extern char *valid_args[];
extern char *frame_name[];

status validate_cmd_args(FILE **fs, int argc, char *argv[])
{
	int idx;

	/* Validation of command line option */
	if (argc < 2)
	{
		printf("\nERROR : Invalid number of arguments\n");
		my_exit(&argv[0]);
		return e_failure;
	}

	/* Validation of command line option */
	for (idx = 0; strcmp(valid_args[idx], "") != 0; idx++)
		if (strcmp(valid_args[idx], argv[1]) == 0)
			break;

	if (strcmp(valid_args[idx], "") == 0)
	{
		printf("\nERROR : Invalid option %s\n", argv[1]);
		my_exit(&argv[0]);
		return e_failure;
	}

	/* Validation of command line option */
	if (strcmp(argv[1], "-h") == 0)
	{
		my_exit(&argv[0]);
		return e_failure;
	}

	/* Check if the command line argument is display */
	if ((strcmp(argv[1], "-d") == 0) || (strcmp(argv[1], "-v") == 0) ||  (strcmp(argv[1], "-p") == 0))
	{
		if (argv[2]  == NULL)
		{
			printf("\nERROR : Missing filename\n");
			my_exit(&argv[0]);
			return e_failure;
		}

		/* Open source file */
		if ((*fs = fopen(argv[2], "r+")) == NULL)
		{
			printf("\nERROR : %s file does not exist\n", argv[2]);
			my_exit(&argv[0]);
			return e_failure;
		}
	}
	else 
	{
		if (argv[3]  == NULL)
		{
			printf("\nERROR : Missing filename\n");
			my_exit(&argv[0]);
			return e_failure;
		}

		/* Open source file */
		if ((*fs = fopen(argv[3], "r+")) == NULL)
		{
			printf("\nERROR : %s file does not exist\n", argv[3]);
			my_exit(&argv[0]);
			return e_failure;
		}
	}
	return e_success;
}

status read_mp3_tag(FILE *fs)
{
	int idx, pos;

	/* Read and store header data */
	fread(header.id, sizeof(char), 3, fs);
	header.id[3] = '\0';
	fread(&header.ver, sizeof(char), 1, fs);

	if (strcmp(header.id, "ID3") != 0)
	{
		printf("\nERROR : Incompatible verison of source file\nERROR : FILE IS NOT ID3.2.3\n\n");
		exit(1);
	}

	if (header.ver != 3)
	{
		printf("\nERROR : Incompatible verison of source file\nERROR : FILE IS NOT ID3.2.3\n\n");
		exit(1);
	}

	fread(&header.rev, sizeof(char), 1, fs);
	fread(&header.flags, sizeof(char), 1, fs);
	fread(&header.tag_size, sizeof(int),  1, fs);
	header.tag_size = convert_syncSafeInteger_to_32BitInteger(convert_bigEndian_to_littleEndian_4Byte(header.tag_size));
	fseek(fs, 0, SEEK_END);
	header.file_size = (unsigned long int) (ftell(fs));
	fseek(fs, 0, SEEK_SET);

	/* Allocate memory to store header.tag_data */
	header.tag_data = (char *) calloc((header.tag_size + SIZE_OF_HEADER), sizeof(char));

	/* Read header data from source file and store in header.tag_data */
	fseek(fs, 0, SEEK_SET);
	fread(header.tag_data, sizeof(char), (header.tag_size + SIZE_OF_HEADER), fs);

	/* Read frames data from mp3 tag */
	for (idx = 0; idx < 7; idx++)
	{
		if ((pos = search_frame(header.tag_data, frame_name[idx], header.tag_size)) != 0)
		{
			frame_t[idx] = (tag_frames *) calloc(1, sizeof(tag_frames));
			strcpy(frame_t[idx]->name, frame_name[idx]);
			frame_t[idx]->name_offset = pos;
			frame_t[idx]->frame_size_offset = frame_t[idx]->name_offset + 4;
			frame_t[idx]->status_flag_offset = frame_t[idx]->frame_size_offset + 4;
			frame_t[idx]->encoding_flag_offset = frame_t[idx]->status_flag_offset + 1;
			frame_t[idx]->encoding_type_offset = frame_t[idx]->encoding_flag_offset + 1;
			frame_t[idx]->frame_data_start_offset = frame_t[idx]->encoding_type_offset + 1;
			fseek(fs, frame_t[idx]->frame_size_offset, SEEK_SET);
			fread(&frame_t[idx]->frame_data_size, sizeof(int), 1, fs);
			frame_t[idx]->frame_data_size =	convert_bigEndian_to_littleEndian_4Byte(frame_t[idx]->frame_data_size);
			frame_t[idx]->frame_data_end_offset = frame_t[idx]->frame_data_start_offset + frame_t[idx]->frame_data_size;
			frame_t[idx]->frame_data = (char *) calloc(frame_t[idx]->frame_data_size + 1, sizeof(char));
			
			/* Read and store frame info */
			fseek(fs, frame_t[idx]->encoding_flag_offset, SEEK_SET);
			fread(&frame_t[idx]->encoding_flag, sizeof(char), 1, fs);

			fseek(fs, frame_t[idx]->status_flag_offset, SEEK_SET);
			fread(&frame_t[idx]->status_flag, sizeof(char), 1, fs);

			fseek(fs, frame_t[idx]->encoding_type_offset, SEEK_SET);
			fread(&frame_t[idx]->encoding_type, sizeof(char), 1, fs);

			fseek(fs, frame_t[idx]->frame_data_start_offset, SEEK_SET);
			fread(frame_t[idx]->frame_data, sizeof(char), frame_t[idx]->frame_data_size - 1, fs);

		}
		else
		{
			/* If frame not found */
			frame_t[idx] = NULL;
		}
	}
	return e_success;
}

/* Display stored information */
status display_mp3_tag(FILE *fs, int argc, char *argv[])
{
	int idx, j;

	if (strcmp(argv[1], "-t") == 0)
	{
		if (frame_t[TIT2] != NULL)
		{
			edit_frame_data(fs, TIT2, argv[2]);
			return e_success;
		}
		else
		{
			create_frame(fs, &frame_t[TIT2], header.tag_data, header.tag_size, TIT2, argv[2]);
			return e_success;
		}
	}
	else if (strcmp(argv[1], "-T") == 0)
	{
		if (frame_t[TRCK] != NULL)
		{
			edit_frame_data(fs, TRCK, argv[2]);
			return e_success;
		}
		else
		{
			create_frame(fs, &frame_t[TRCK], header.tag_data, header.tag_size, TRCK, argv[2]);
			return e_success;
		}
	}
	else if (strcmp(argv[1], "-a") == 0)
	{
		if (frame_t[TPE1] != NULL)
		{
			edit_frame_data(fs, TPE1, argv[2]);
			return e_success;
		}
		else
		{
			create_frame(fs, &frame_t[TPE1], header.tag_data, header.tag_size, TPE1, argv[2]);
			return e_success;
		}
	}
	else if (strcmp(argv[1], "-A") == 0)
	{
		if (frame_t[TALB] != NULL)
		{
			edit_frame_data(fs, TALB, argv[2]);
			return e_success;
		}
		else
		{
			create_frame(fs, &frame_t[TALB], header.tag_data, header.tag_size, TALB, argv[2]);
			return e_success;
		}
	}
	else if (strcmp(argv[1], "-y") == 0)
	{
		if (frame_t[TYER] != NULL)
		{
			edit_frame_data(fs, TYER, argv[2]);
			return e_success;
		}
		else
		{
			create_frame(fs, &frame_t[TYER], header.tag_data, header.tag_size, TYER, argv[2]);
			return e_success;
		}
	}
	else if (strcmp(argv[1], "-c") == 0)
	{
		if (frame_t[COMM] != NULL)
		{
			edit_frame_data(fs, COMM, argv[2]);
			return e_success;
		}
		else
		{
			create_frame(fs, &frame_t[COMM], header.tag_data, header.tag_size, COMM, argv[2]);
			return e_success;
		}
	}
	else if (strcmp(argv[1], "-v") == 0)
	{
		printf("\nVersion %s.%d.%d\n", header.id, header.ver, header.rev);
		return e_success;
	}
	else if (strcmp(argv[1], "-p") == 0)
	{
		if (frame_t[APIC] != NULL)
			find_image(frame_t[APIC]->frame_data, frame_t[APIC]->frame_data_size);
		else
			printf("Image not found\n");
		return e_success;
	}
	else if (strcmp(argv[1], "-d") == 0)
	{
		puts("******************************");
		for (idx = 0; idx < 6; idx++)
		{
			if (frame_t[idx] != NULL)
			{
				switch (idx)
				{
				
					case TALB: 
						printf("\nAlbum : %s\n", frame_t[idx]->frame_data);
						break;
					case TIT2: 
						printf("\nTitle Track : %s\n", frame_t[idx]->frame_data);
						break;
					case TYER: 
						printf("\nYear : %s\n", frame_t[idx]->frame_data);
						break;
					case TRCK: 
						printf("\nTrack : %s\n", frame_t[idx]->frame_data);
						break;
					case TPE1: 
						printf("\nArtist : %s\n",	frame_t[idx]->frame_data);
						break;
					case COMM: 
						printf("\nComments : ");
						for (j = 0; (j != frame_t[idx]->frame_data_size); j++)
							putchar(frame_t[idx]->frame_data[j]);
						puts("\n");		
						break;
				}
			}
		}
		puts("******************************");
		return e_success;
	}
	else
	{
		printf("\nERROR : Unknown command line option\n");
		my_exit(&argv[0]);
		return e_failure;
	}
}

unsigned int search_frame(char *tag_data, char *frame_name, unsigned int tag_size)
{
	int i, j, k;

	for (i = 0; i < tag_size; i++)
	{
		if (tag_data[i] == frame_name[0])
		{
			for (j = i, k = 0; frame_name[k] != '\0'; j++, k++)
			{
				if (tag_data[j] != frame_name[k])
					break;
			}
			if (frame_name[k] == '\0')
				return i;
		}
	}
	return 0;
}

unsigned int convert_bigEndian_to_littleEndian_4Byte(unsigned int big_endian)
{
	unsigned int little_endian = 0;

	little_endian |= ((big_endian & 0x000000FF) << 24);
	little_endian |= ((big_endian & 0x0000FF00) <<  8);
	little_endian |= ((big_endian & 0x00FF0000) >>  8);
	little_endian |= ((big_endian & 0xFF000000) >> 24);

	return little_endian;
}
unsigned int convert_littleEndian_to_bigEndian_4Byte(unsigned int little_endian)
{
	unsigned int big_endian = 0;

	big_endian |= ((little_endian & 0x000000FF) << 24);
	big_endian |= ((little_endian & 0x0000FF00) <<  8);
	big_endian |= ((little_endian & 0x00FF0000) >>  8);
	big_endian |= ((little_endian & 0xFF000000) >> 24);

	return big_endian;
}

unsigned int convert_33BitInteger_to_syncSafeInteger(unsigned int integer_32_bit)
{
	unsigned int sync_safe_integer = 0;
	unsigned int a, b, c, d;

	a = integer_32_bit & 0x7F;
	b = ((integer_32_bit >> 7) & 0x7F);
	c = ((integer_32_bit >> 14) & 0x7F);
	d = ((integer_32_bit >> 21) & 0x7F);

	sync_safe_integer |= a;
	sync_safe_integer |= b << 8;
	sync_safe_integer |= c << 16;
	sync_safe_integer |= d << 24;

	return sync_safe_integer;
}

unsigned int convert_syncSafeInteger_to_32BitInteger(unsigned int sync_safe_integer)
{
	unsigned int integer_32_bit = 0;
	unsigned int a, b, c, d;

	a = sync_safe_integer & 0xFF;
	b = ((sync_safe_integer >> 8) & 0xFF);
	c = ((sync_safe_integer >> 16) & 0xFF);
	d = ((sync_safe_integer >> 24) & 0xFF);

	integer_32_bit |= a;
	integer_32_bit |= b << 7;
	integer_32_bit |= c << 14;
	integer_32_bit |= d << 21;

	return integer_32_bit;
}

unsigned int mem_alloc(char *tag_data, unsigned int tag_size, unsigned int size)
{
	unsigned int idx, j, no_of_bytes_available;

	for (idx = 0; idx < tag_size; idx++)
	{
		if (tag_size - idx < size)
		{
			/* Memory not avilable -> Error handling in calling function */
			return 0;
		}

		no_of_bytes_available = 0;
		if (tag_data[idx] == '\0')
		{
			for (j = idx; (j <= idx + size) && (j < tag_size); j++)
			{
				if (tag_data[j] == '\0')
					no_of_bytes_available++;
				else
					break;
			}
			if (no_of_bytes_available >= size)
				break;
		}
	}
	if (no_of_bytes_available >= size)
		return idx;
	else 
		return 0;
}

status edit_frame_data(FILE * fs, FRAME_NAME frameName, char *new_data)
{
	int idx;

		for (idx = 0; idx < frame_t[frameName]->frame_data_size - 1; idx++)
			frame_t[frameName]->frame_data[idx] = '\0';

		strcpy(frame_t[frameName]->frame_data, new_data);

		fseek(fs, frame_t[frameName]->frame_data_start_offset, SEEK_SET);
		fwrite(frame_t[frameName]->frame_data, sizeof(char), frame_t[frameName]->frame_data_size - 1, fs);
		return e_success;
}

status create_frame(FILE * fs, tag_frames ** frame_t, char *tag_data, int tag_size, FRAME_NAME frameName, char *new_data)
{
	int idx;

		(*frame_t) = (tag_frames *) calloc(1, sizeof(tag_frames));
		strcpy((*frame_t)->name, frame_name[frameName]);

		if (((*frame_t)->name_offset = mem_alloc(tag_data, tag_size, 1000)) == 0)
		{
			printf("Memory not available. Requested frame cannot be created\n");
			free(*frame_t);
			(*frame_t) = NULL;
			return e_failure;
		}

		(*frame_t)->status_flag = 0;
		(*frame_t)->encoding_flag = 0;
		(*frame_t)->encoding_type = 0;

		(*frame_t)->frame_data = (char *) calloc(20, sizeof(char));
		strcpy((*frame_t)->frame_data, new_data);

		(*frame_t)->frame_data_size = convert_littleEndian_to_bigEndian_4Byte(21);

		fseek(fs, (*frame_t)->name_offset, SEEK_SET);
		fwrite((*frame_t)->name, sizeof(char), 4, fs);
		fwrite(&(*frame_t)->frame_data_size , sizeof(int), 1, fs);
		fwrite(&(*frame_t)->status_flag, sizeof(char), 1, fs);
		fwrite(&(*frame_t)->encoding_flag, sizeof(char), 1, fs);
		fwrite(&(*frame_t)->encoding_type, sizeof(char), 1, fs);
		fwrite((*frame_t)->frame_data, sizeof(char), 20, fs);

	return e_success;
}

status find_image(char *tag_data, unsigned int tag_size)
{
	int i, j, k, pos, image_size;
	char *image_buf;
	FILE *im = NULL;

	pos = search_frame(tag_data, "APIC", tag_size);
	if ((pos = search_frame(tag_data, "jpeg", tag_size)) != 0)
	{
		pos = pos + 6;
		for (j = pos; tag_data[j] != '\0'; j++);
		image_size = tag_size - j;
		putchar(tag_data[j + 1]);

		image_buf = (char *) calloc(image_size, sizeof(char));

		for (k = j + 1, i = 0; k < tag_size; k++, i++)
			image_buf[i] = tag_data[k];
		
		if ((im = fopen("img.jpg", "w")) != NULL)
		{
			fwrite(image_buf, image_size, 1, im);
			fclose(im);
            system("eog img.jpg");
			free(image_buf);
			return e_success;
		}
		else
		{
			printf("ERROR : Unable to open file %s\n", "img.jpg");
			return e_failure;
		}
	}
	else if ((pos = search_frame(tag_data, "png", tag_size)) != 0)
	{
		pos = pos + 5;
		for (j = pos; tag_data[j] != '\0'; j++);
		image_size = tag_size - j;
		putchar(tag_data[j + 1]);

		image_buf = (char *) calloc(image_size, sizeof(char));

		for (k = j + 1, i = 0; k < tag_size; k++, i++)
			image_buf[i] = tag_data[k];
		
		if ((im = fopen("img.png", "w")) != NULL)
		{
			fwrite(image_buf, image_size, 1, im);
			fclose(im);
            system("eog img.png");
			free(image_buf);
			return e_success;
		}
		else
		{
			printf("ERROR : Unable to open file %s\n", "img.jpg");
			return e_failure;
		}
	}
	else
	{
		printf("Image not found\n");
		return e_failure;
	}
}

/* Error handling function for program usage error*/
status my_exit(char **cmd_args)
{
	fprintf(stderr, "\n$ mp3tag --help \n"); 
	fprintf(stderr, "Usage : %s -[tTaAycg] [Data] [FILE NAME]\n", *cmd_args);
	fprintf(stderr, "        %s -v [FILE NAME]\n", *cmd_args);
	fprintf(stderr, "        %s -d [FILE NAME]\n\n", *cmd_args);
	fprintf(stderr, "   -t   :    Modifies a Title tag\n");
	fprintf(stderr, "   -T   :    Modifies a Track tag\n");
	fprintf(stderr, "   -a   :    Modifies a Artist tag\n");
	fprintf(stderr, "   -A   :    Modifies a Album tag\n");
	fprintf(stderr, "   -y   :    Modifies a Year tag\n");
	fprintf(stderr, "   -c   :    Modifies a Comment tag\n");
	fprintf(stderr, "   -h   :    Displays this help info\n");
	fprintf(stderr, "   -v   :    Displays Version info\n");
	fprintf(stderr, "   -d   :    Displays all tags\n\n");

    return e_success;
}
