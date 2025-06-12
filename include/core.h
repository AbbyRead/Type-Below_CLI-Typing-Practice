#ifndef CORE_H
#define CORE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_MATCH 0
#define CHUNK_SIZE 4096

enum InputMode {
	INPUT_MODE_FILE,
	INPUT_MODE_PIPE
};

void echo_usage(const char *prog_name);
void precheck_arguments(int argc, char *argv[]);
enum InputMode determine_input_mode(const char *file_arg);
char *copy_to_buffer(FILE *stream);
long count_lines(const char *buffer);
long validate_line_number(const char *line_number_str, const long total_lines);
void set_starting_offset(const char *buffer, unsigned long *offset, const long starting_line);
int get_terminal_height(void);
void move_cursor_up(int lines);
int print_next_line(const char *buffer, unsigned long *offset, size_t buffer_length);

#endif
