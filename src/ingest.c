#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "ingest.h"
#include "platform.h"
#include "version.h"

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
		if (c == EOF) {
			if (ferror(stream)) {
				perror("Error reading input");
				free(buffer);
				return NULL;
			}
			break;
		}
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
		free(buffer);
		return NULL;
	}

	// printf("Local buffer allocated bytes: %zu\n", byte_count + 1);

	return final_buffer;
}

char *copy_from_clipboard() {
	#ifdef _WIN32
		if (!OpenClipboard(NULL)) return NULL;
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData == NULL) {
			CloseClipboard();
			return NULL;
		}
		char *clipboard_data = GlobalLock(hData);
		if (!clipboard_data) {
			CloseClipboard();
			return NULL;
		}
		char *copy = strdup(clipboard_data);
		GlobalUnlock(hData);
		CloseClipboard();
		return copy;
	#else
		FILE *pipe = popen(
			#ifdef __APPLE__
				"pbpaste"
			#else
				"xclip -selection clipboard -o"
			#endif
		, "r");

		if (!pipe) {
			perror("Could not open clipboard pipe");
			return NULL;
		}
		char *buffer = copy_to_buffer(pipe);
		pclose(pipe);
		return buffer;
	#endif
}

long count_lines(const char *buffer) {
	if (buffer[0] == '\0') return 0; // Fix: empty buffer = 0 lines
	// The final (or potentially only) line may be \0 terminated.
	long lines = 1;
	while (*buffer != '\0') {
		if (*buffer++ == '\n') lines++;
	}
	return lines;
}


void precheck_arguments(int argc, char *argv[]) {
	if (argc >= 2 && 
		(strcmp(argv[1], "--help") == STRING_MATCH || strcmp(argv[1], "-h") == STRING_MATCH)) {
		echo_usage(argv[0]);
		exit(EXIT_SUCCESS);
	}
	if (argc >= 2 && 
		(strcmp(argv[1], "--version") == STRING_MATCH || strcmp(argv[1], "-v") == STRING_MATCH)) {
		printf("TypeBelow: Version %s\n", PROGRAM_VERSION);
		exit(EXIT_SUCCESS);
	}
	if (argc > 3) {
		fprintf(stderr, "Too many arguments provided.\n");
		echo_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
}

enum InputMode determine_input_mode(const char *file_arg) {
	if (!file_arg) return INPUT_MODE_CLIPBOARD;
	return strcmp(file_arg, "-") == STRING_MATCH ? INPUT_MODE_PIPE : INPUT_MODE_FILE;
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