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

long validate_line_number(const char *line_number_str) {
	long line_number = atol(line_number_str);
	// To Do: make negative numbers backtrack from the end.
	return line_number;
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
	printf("%x\n", buffer[i]);

	buffer = realloc(buffer, byte_count);
	if (!buffer) exit(EXIT_FAILURE);
	return buffer;
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
		starting_line = validate_line_number(argv[2]);
	}
	printf("Reading from '%s', starting from line %ld.\n", argv[1], starting_line);

	// Print the whole buffer for now to verify it's working
	printf("%s\n", buffer);

	// To Do: Implement line-by-line printing with user prompting.

	return 0;
}
