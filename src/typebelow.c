#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_MATCH 0
#define CHUNK_SIZE 4096

// Operating mode enums: piped input or file input
enum InputMode {
	INPUT_MODE_FILE,
	INPUT_MODE_PIPE
};

void echo_usage(const char *prog_name) {
	fprintf(stderr, "Usage: %s <filename> [starting_line]\n", prog_name);
}

void precheck_arguments(int argc, char *argv[]) {
	if (argc < 2) { // At least one argument is required (the filename)
		fprintf(stderr, "Expected a filename or '-' to specify source text.\n");
		echo_usage(*argv);
		exit(EXIT_FAILURE);
	}
	if (argc > 3) { // Allow at most two arguments: filename and optional starting line
		fprintf(stderr, "Too many arguments provided.\n");
		echo_usage(*argv);
		exit(EXIT_FAILURE);
	}
	if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == STRING_MATCH) {
		echo_usage(*argv);
		exit(EXIT_SUCCESS);
	}
}

enum InputMode determine_input_mode(const char *file_arg) {
	if (strcmp(file_arg, "-") == STRING_MATCH) {
		return INPUT_MODE_PIPE;
	} else {
		return INPUT_MODE_FILE;
	}
}

char *copy_to_buffer(FILE *stream) {
	if (!stream) exit(EXIT_FAILURE);

	size_t bytes_available = CHUNK_SIZE;
	size_t byte_count = 0;
	size_t i = 0;
	char *buffer = malloc(bytes_available);
	if (!buffer) exit(EXIT_FAILURE);

	while (!feof(stream) && !ferror(stream)) {
		buffer[i] = (char)getc(stream);
		i++;
		if (i == bytes_available) {
			bytes_available += CHUNK_SIZE;
			buffer = realloc(buffer, bytes_available);
			if (!buffer) exit(EXIT_FAILURE);
		}
	}
	buffer[i - 1] = '\0';
	byte_count = i;

	buffer = realloc(buffer, byte_count);
	if (!buffer) exit(EXIT_FAILURE);
	return buffer;
}

long count_lines(const char *buffer) {
	char c;
	long lines = 1;
	size_t i = 0;
	do {
		c = buffer[i];
		if (c == '\n') lines++;
		i++;
	} while (c != '\0');
	return lines;
}

long validate_line_number(const char *buffer, const char *line_number_str) {
	long line_number = atol(line_number_str);
	long lines_available = count_lines(buffer);

	if (line_number > lines_available) {
		printf("Starting line specified: %ld"
			" is greater than number of lines available: %ld.\n", line_number, lines_available);
		exit(EXIT_FAILURE);
	}
	if (line_number < 0) {
		line_number += lines_available;
		if (line_number < 1) {
			printf("%s\n", "Starting line offset from end is greater than the total number of lines.");
			printf("Total: %ld\tSpecified: %s\nWhich would evaluate as line %ld.\n", lines_available, line_number_str, line_number);
			exit(EXIT_FAILURE);
		}
	}
	return line_number;
}

int print_next_line(const char *buffer, long *offset) {
	long i = *offset;
	char c;
	do {
		c = buffer[i];
		if (c == '\0') return 1;
		putchar((int)c);
		i++;
	} while (c != '\n');
	*offset = i;
	return 0;
}

int main(int argc, char *argv[]) {
	precheck_arguments(argc, argv);

	enum InputMode mode;
	FILE *source_text = NULL;
	mode = determine_input_mode(argv[1]);

	switch (mode) {
		case INPUT_MODE_PIPE:
			printf("Reading from stdin...\n");
			source_text = stdin;
			printf("Input mode: Piped input (stdin)\n");
			break;
		case INPUT_MODE_FILE: 
			printf("Reading from file...\n");
			source_text = fopen(argv[1], "r");
			if (!source_text) {
				fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
				exit(EXIT_FAILURE);
			}
			printf("Input mode: File\n");
			break;
		default:
			fprintf(stderr, "Unknown input mode.\n");
			echo_usage(*argv);
			exit(EXIT_FAILURE);
			break;
	}
	
	char *buffer = copy_to_buffer(source_text);

	long starting_line = 1;
	if (argc == 3) { // If a starting line is provided as the second argument
		starting_line = validate_line_number(buffer, argv[2]);
	}
	printf("Reading from '%s', starting from line %ld.\n", argv[1], starting_line);

	long offset = 0;
	while (!print_next_line(buffer, &offset)){
		// Replace this part with an actual input routine:
		printf("%s\n", "USER INPUT\n");
	}
	return 0;
}
