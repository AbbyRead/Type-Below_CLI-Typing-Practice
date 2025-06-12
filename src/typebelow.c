#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "version.h"
#include "platform.h"

#define STRING_MATCH 0
#define CHUNK_SIZE 4096

enum InputMode {
	INPUT_MODE_FILE,
	INPUT_MODE_PIPE
};

void echo_usage(const char *prog_name) {
	fprintf(stderr,
		"Usage:\n"
		"  %s <filename> [starting_line]\n"
		"  %s - [starting_line]\n\n"
		"Options:\n"
		"  -h, --help          Show this help message and exit.\n\n"
		"Examples:\n"
		"  %s myfile.txt            # Read from file starting at line 1\n"
		"  %s myfile.txt 10         # Start at line 10\n"
		"  cat myfile.txt | %s -    # Read from piped stdin\n"
		"  cat myfile.txt | %s - -2 # Start 2 lines from end\n\n",
		prog_name, prog_name, prog_name, prog_name, prog_name, prog_name);
}

void precheck_arguments(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Error: Missing filename or '-'.\n");
		fprintf(stderr, "Tip: Try '%s --help' for usage info.\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if (argc > 3) {
		fprintf(stderr, "Too many arguments provided.\n");
		echo_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	if (strcmp(argv[1], "--help") == STRING_MATCH || strcmp(argv[1], "-h") == STRING_MATCH) {
		echo_usage(argv[0]);
		exit(EXIT_SUCCESS);
	}
	if (strcmp(argv[1], "--version") == STRING_MATCH || strcmp(argv[1], "-v") == STRING_MATCH) {
		printf("TypeBelow: Version %s\n", PROGRAM_VERSION);
		exit(EXIT_SUCCESS);
	}
}

enum InputMode determine_input_mode(const char *file_arg) {
	return strcmp(file_arg, "-") == STRING_MATCH ? INPUT_MODE_PIPE : INPUT_MODE_FILE;
}

char *copy_to_buffer(FILE *stream) {
	if (!stream) {
		perror("Invalid stream");
		return NULL;
	}

	size_t bytes_available = CHUNK_SIZE;
	size_t byte_count = 0;
	char *buffer = malloc(bytes_available);
	if (!buffer) {
		perror("Failed to allocate memory");
		return NULL;
	}

	while (1) {
		int c = getc(stream);
		if (c == EOF || ferror(stream)) break;
		buffer[byte_count++] = (char)c;
		if (byte_count == bytes_available) {
			bytes_available += CHUNK_SIZE;
			char *new_buffer = realloc(buffer, bytes_available);
			if (!new_buffer) {
				perror("Failed to reallocate memory");
				free(buffer);
				return NULL;
			}
			buffer = new_buffer;
		}
	}
	buffer[byte_count] = '\0';

	char *final_buffer = realloc(buffer, byte_count + 1);
	if (!final_buffer) {
		perror("Failed to reallocate final memory");
		return NULL;
	}

	printf("Local buffer allocated bytes: %zu\n", byte_count + 1);

	return final_buffer;
}

long count_lines(const char *buffer) {
	long lines = 1;
	while (*buffer != '\0') {
		if (*buffer++ == '\n') lines++;
	}
	return lines;
}

long validate_line_number(const char *line_number_str, const long total_lines) {
	errno = 0;
	char *endptr;
	long line_number = strtol(line_number_str, &endptr, 10);

	if (errno != 0 || *endptr != '\0') {
		fprintf(stderr, "Invalid starting line number: %s\n", line_number_str);
		exit(EXIT_FAILURE);
	}

	if (line_number > total_lines) {
		printf("Starting line specified: %ld is greater than number of lines available: %ld.\n",
			line_number, total_lines);
		exit(EXIT_FAILURE);
	}
	if (line_number < 0) {
		line_number += total_lines;
		if (line_number < 1) {
			printf("Starting line offset from end is greater than the total number of lines.\n");
			printf("Total: %ld\tSpecified: %s\nWhich would evaluate as line %ld.\n",
				total_lines, line_number_str, line_number);
			exit(EXIT_FAILURE);
		}
	}
	return line_number;
}

void set_starting_offset(const char *buffer, unsigned long *offset, const long starting_line) {
	long remaining_searches = starting_line - 1;
	const char *substring = buffer + *offset;

	while (remaining_searches > 0) {
		const char *newline = strchr(substring, '\n');
		if (!newline) break;
		*offset = (unsigned long)(newline - buffer + 1);
		substring = newline + 1;
		--remaining_searches;
	}
}

int get_terminal_height() {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns, rows;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return rows;
#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_row;
#endif
}

void move_cursor_up(int lines) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos;

    GetConsoleScreenBufferInfo(hOut, &csbi);
    pos.X = 0;
    pos.Y = csbi.dwCursorPosition.Y - lines;
    SetConsoleCursorPosition(hOut, pos);
#else
    printf("\033[%dA", lines);  // ANSI escape: move cursor up
#endif
}

int print_next_line(const char *buffer, unsigned long *offset, size_t buffer_length) {
	if (*offset >= buffer_length) return 1;

	const char *line_start = &buffer[*offset];
	const char *newline_pos = strchr(line_start, '\n');
	size_t line_length = newline_pos ? (newline_pos - line_start + 1) : (buffer_length - *offset);

	fwrite(line_start, 1, line_length, stdout);
	*offset += line_length;
	return (*offset >= buffer_length);
}

int main(int argc, char *argv[]) {
	precheck_arguments(argc, argv);

	enum InputMode mode = determine_input_mode(argv[1]);
	FILE *source_text = NULL;

	switch (mode) {
		case INPUT_MODE_PIPE:
			if (isatty(fileno(stdin))) {
				fprintf(stderr, "Error: No piped-in text detected on stdin.\n");
				return EXIT_FAILURE;
			}
			source_text = stdin;
			printf("Input mode: Piped input (stdin)\n");
			break;
		case INPUT_MODE_FILE:
			source_text = fopen(argv[1], "r");
			if (!source_text) {
				fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
				return EXIT_FAILURE;
			}
			printf("Input mode: File\n");
			break;
		default:
			fprintf(stderr, "Unknown input mode.\n");
			echo_usage(argv[0]);
			return EXIT_FAILURE;
	}

	char *buffer = copy_to_buffer(source_text);
	if (!buffer) {
		fclose(source_text);
		return EXIT_FAILURE;
	}
	fclose(source_text);

	FILE *user_input_stream = fopen(USER_INPUT_DEVICE, "r");
	if (!user_input_stream) {
		fprintf(stderr, "Error: Could not open terminal for user input.\n");
		free(buffer);
		return EXIT_FAILURE;
	}

	long total_lines = count_lines(buffer);
	long starting_line = 1;
	unsigned long offset = 0;
	if (argc == 3) {
		starting_line = validate_line_number(argv[2], total_lines);
		set_starting_offset(buffer, &offset, starting_line);
	}

	printf("Reading from '%s', starting from line %ld of %ld.\n", argv[1], starting_line, total_lines);
	long line = starting_line;
	char user_input[1024];
	size_t buffer_length = strlen(buffer);
	int last_line;

	while (line < total_lines + 1) {
		last_line = print_next_line(buffer, &offset, buffer_length);
		if (last_line) printf("\n");

		// Position the text input line
		int height = get_terminal_height();
		float padding_ratio = 0.4; 
		int pad_lines = (int)(height * padding_ratio);
		for (int i = 0; i < pad_lines; i++) {
			printf("\n");
		}
		move_cursor_up(pad_lines);
		fflush(stdout);

		if (fgets(user_input, sizeof(user_input), user_input_stream)) {
			user_input[strcspn(user_input, "\n")] = '\0';
		} else {
			printf("Program ended on line %ld of %ld\n", line, total_lines);
			printf("To continue from this point next time use the command:\n");
			printf("%s %s %ld\n", argv[0], argv[1], line);
			free(buffer);
			fclose(user_input_stream);
			return EXIT_FAILURE;
		}
		printf("\n");
		line++;
	}

	free(buffer);
	fclose(user_input_stream);
	return 0;
}
