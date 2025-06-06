#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

char *read_entire_stream(FILE *file) {
	size_t capacity = 4096;  // Start with 4KB, grow as needed
	size_t length = 0;
	char *buffer = malloc(capacity);
	if (!buffer) {
		fprintf(stderr, "Memory allocation failed.\n");
		return NULL;
	}

	int c;
	while ((c = fgetc(file)) != EOF) {
		if (length + 1 >= capacity) {
			capacity *= 2;
			char *new_buffer = realloc(buffer, capacity);
			if (!new_buffer) {
				free(buffer);
				fprintf(stderr, "Memory reallocation failed.\n");
				return NULL;
			}
			buffer = new_buffer;
		}
		buffer[length++] = (char)c;
	}

	buffer[length] = '\0'; // Null-terminate
	return buffer;
}

int echo_usage(const int argc, const char *prog_name) {
	if (argc > 3) fprintf(stderr, "Too many arguments provided.\n");
	fprintf(stderr, "Usage: %s <filename> [starting_line]\n", prog_name);
	fprintf(stderr, "   Or: %s [starting_line]\n", prog_name);
	fprintf(stderr, "   (if piping data into the program).\n");
	return 1;
}

unsigned long validate_line_number(const char *line_number_str) {
	char *end;
	errno = 0; // Reset errno before strtoul
	unsigned long line_number = strtoul(line_number_str, &end, 10);
	if (errno != 0 || *end != '\0' || line_number < 1) {
		fprintf(stderr, "Invalid starting line number: %s\n", line_number_str);
		return 0;
	}
	return line_number;
}

// Main function to read a file or stdin and print lines one by one
int main(int argc, char *argv[]) {
	FILE *file = NULL;
	unsigned long starting_line = 1;
	if (argc > 3) { // more than program name, filename, and starting line
		return echo_usage(argc, *argv);
	}
	// Deal with any piped in data as preferred over specifying a file
	// Avoid using stdin if it is a terminal (tty) and no filename is provided
	if (!isatty(fileno(stdin))) {
		switch (argc) {
			case 2: // program name and starting line only
				starting_line = validate_line_number(argv[1]);
				// fallthrough
			case 1: // program name only
				// No starting line specified, read from stdin
				file = stdin;
				break;
			default:
				return echo_usage(argc, *argv); // End program with usage message
		}
	} else { // If stdin is a terminal, we expect a filename to be used as input
		switch (argc) {
			case 2:
				// If only one argument is provided, treat it as a filename
				file = fopen(argv[1], "r");
				if (!file) {
					fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
					return 1;
				}
				break;
			case 3:
				// If two arguments are provided, treat the first as a filename and the second as a starting line
				file = fopen(argv[1], "r");
				if (!file) {
					fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
					return 1;
				}
				char *end;
				errno = 0; // Reset errno before strtoul
				starting_line = strtoul(argv[2], &end, 10);
				if (errno != 0 || *end != '\0' || starting_line < 1) {
					fprintf(stderr, "Invalid starting line number: %s\n", argv[2]);
					fclose(file);
					return echo_usage(argc, *argv);
				}
				break;
		}
	}

	char *file_contents = read_entire_stream(file);
	// Check if stream failed to copy
	if (!file_contents) {
		if (file != stdin) fclose(file);
		printf("%s\n", "Unable to copy stream.");
		return 1;
	} 
	// file_contents should now be a copy of the stream (from stdin or actual file)
	// This is a dynamically-allocated buffer created from the read_entire_stream function.

	char *line_start = file_contents;
	char *newline_pos;
	char input_buf[120]; // characters per line

	// while character search of file_contents does not return ending NULL
	while ((newline_pos = strchr(line_start, '\n')) != NULL) {
		*newline_pos = '\0'; // Temporarily insert a NULL to end on the end of line
		printf("\n%s\n", line_start);  // Print one line from file_contents

		// Wait for user to press Enter
		if (!fgets(input_buf, sizeof(input_buf), stdin)) {
			printf("\nInput error or EOF. Exiting.\n");
			break;
		}

		*newline_pos = '\n';          // Restore newline character
		line_start = newline_pos + 1; // Start on next line now
	}

	// Print last line if any (no trailing newline)
	if (*line_start != '\0') {
		printf("%s\n", line_start);
	}

	free(file_contents);
	return 0;
}
