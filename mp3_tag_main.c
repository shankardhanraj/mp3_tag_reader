#include "mp3_tag.h"

/* Create a structure for header */
tag_header header;

/* Array of structure pointers to store tag_frames*/
tag_frames *frame_t[6];

/* Name of tag_frames */
char *frame_name[] = {"TALB", "TIT2", "TYER", "TRCK", "TPE1", "COMM", "APIC"};

/* Valid command line arguments */
char *valid_args[] = {"-t", "-T", "-a", "-A", "-c", "-y", "-v", "-d", "-h", "-p", ""};

/* Function Prototypes */
status validate_cmd_args(FILE **fs, int argc, char *argv[]);
status read_mp3_tag(FILE *fs);
status display_mp3_tag(FILE *fs, int argc, char *argv[]);

status main(int argc, char *argv[])
{
	int idx, j;
	FILE *fs;

	/* validate_cmd_args */
	if (validate_cmd_args(&fs, argc, argv) == e_failure)
		return e_failure;


	/* read_mp3_tags */
	if (read_mp3_tag(fs) == e_failure)
		return e_failure;

	/* display_mp3_tags */
	if (display_mp3_tag(fs, argc, argv) == e_failure)
		return e_failure;

	/* close mp3 file */
	fclose(fs);

    return e_success;
}
