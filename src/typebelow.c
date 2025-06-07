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

int check_for_content(FILE *stream) {
	// Check if the stream is empty
	int c = fgetc(stream);
	if (c == EOF) {
		return 1; // Stream is empty
	}
	ungetc(c, stream); // Put the character back for further processing
	return 0; // Stream has content
}
	

char *read_entire_stream(FILE *file, size_t *capacity) {
	*capacity = 4096;  // Start with 4KB, grow as needed
	char *buffer = malloc(*capacity);
	if (!buffer) {
		fprintf(stderr, "Memory allocation failed.\n");
		return NULL;
	}
	int c;
	size_t i = 0; // Index for the buffer
	do {
		c = fgetc(file);
		if (i + 1 >= *capacity) {
			*capacity += 4096; // Increase capacity by 4KB
			char *new_buffer = realloc(buffer, *capacity);
			if (!new_buffer) {
				free(buffer);
				fprintf(stderr, "Memory reallocation failed.\n");
				return NULL;
			}
			buffer = new_buffer;
		}
		buffer[i] = (char)c;
		i++;
	} while (c != EOF);
	buffer[i] = '\0'; // Null-terminate
	return buffer;
}

void echo_usage(const int argc, const char *prog_name) {
	if (argc > 3) fprintf(stderr, "Too many arguments provided.\n");
	fprintf(stderr, "Usage: %s <filename> [starting_line]\n", prog_name);
}

unsigned long validate_line_number(const char *line_number_str) {
	char *end;
	errno = 0; // Reset errno before strtoul
	unsigned long line_number = strtoul(line_number_str, &end, 10);
	if (errno != 0 || *end != '\0' || line_number < 1) {
		return 0;
	}
	return line_number;
}

// Main function to read a file or stdin and print lines one by one
int main(int argc, char *argv[]) {
	FILE *file = NULL;
	unsigned long starting_line = 1;

	if (argc > 3) {
		echo_usage(argc, *argv);
		return 1;
	}

	bool piped = !isatty(fileno(stdin));

	if (piped) {
		if (argc == 2) starting_line = validate_line_number(argv[1]);
		if (starting_line == 0) {
			fprintf(stderr, "Invalid starting line number: %s\n", argv[1]);
			echo_usage(argc, *argv);
			return 1;
		}
		if (check_for_content(stdin)) {
			fprintf(stderr, "No content provided via stdin.\n");
			echo_usage(argc, *argv);
			return 1;
		}
		file = stdin;
	} else {
		// If not piped, check for a filename argument
		if (argc < 2) {
			fprintf(stderr, "Expected a filename as input.\n");
			echo_usage(argc, *argv);
			return 1;
		}
		file = fopen(argv[1], "r");
		if (!file) {
			fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
			return 1;
		}
		if (argc == 3) {
			starting_line = validate_line_number(argv[2]);
		}
	}
	if (!file) { // If file is still NULL, it means we didn't open stdin or a file
		echo_usage(argc, *argv);
		return 1;
	}

	size_t capacity = 0;
	char *file_contents = read_entire_stream(file, &capacity);
	// Check if stream failed to copy
	if (!file_contents) {
		if (file != stdin) fclose(file);
		printf("%s\n", "Unable to copy stream.");
		return 1;
	} 
	// Close the file now that its contents have been copied
	if (file != stdin) fclose(file);

	char *line_start = file_contents;
	char *newline_pos;

	// Open terminal input for user interaction, separate from piped stdin
	FILE *user_input = fopen(
	#ifdef _WIN32
		"CON"
	#else
		"/dev/tty"
	#endif
	, "r");

	if (!user_input) {
		fprintf(stderr, "Could not open terminal for user input.\n");
		free(file_contents);
		return 1;
	}

	// while character search of file_contents does not return ending NULL
	while ((newline_pos = strchr(line_start, '\n')) != NULL) {
		*newline_pos = '\0'; // Temporarily insert a NULL to end on the end of line
		printf("\n%s\n", line_start);  // Print one line from file_contents
		char buffer[512];
		if (!fgets(buffer, sizeof(buffer), stdin)) {
			fprintf(stderr, "Error reading from user input: %s\n", strerror(errno));
			fclose(user_input);
			free(file_contents);
			return 1;
		}
		buffer[strcspn(buffer, "\n")] = '\0';  // Strip newline

		
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
