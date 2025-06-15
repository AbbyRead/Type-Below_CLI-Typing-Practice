#ifndef INGEST_H
#define INGEST_H

#define STRING_MATCH 0
#define CHUNK_SIZE 4096

enum InputMode {
	INPUT_MODE_FILE,
	INPUT_MODE_PIPE,
	INPUT_MODE_CLIPBOARD
};

char *copy_to_buffer(FILE *stream);
char *copy_from_clipboard();
long count_lines(const char *buffer);
void precheck_arguments(int argc, char *argv[]);
enum InputMode determine_input_mode(const char *file_arg);
long validate_line_number(const char *line_number_str, const long total_lines);
void set_starting_offset(const char *buffer, unsigned long *offset, const long starting_line);

#endif