#include "core.h"
#include "ingest.h"
#include "platform.h"
#include "version.h"

int main(int argc, char *argv[]) {
	precheck_arguments(argc, argv);

	enum InputMode mode = determine_input_mode(argv[1]);
	FILE *source_text = NULL;

	switch (mode) {
		case INPUT_MODE_PIPE:
			#ifdef _WIN32
				// On Windows, _isatty() often returns true even with piped input,
				// so we skip this check and assume the user knows what they're doing.
				source_text = stdin;
			#else
				if (isatty(fileno(stdin))) {
					fprintf(stderr, "Error: No piped-in text detected on stdin.\n");
					return EXIT_FAILURE;
				}
				source_text = stdin;
			#endif
			// printf("Input mode: Piped input (stdin)\n");
			break;
		case INPUT_MODE_FILE:
			source_text = fopen(argv[1], "r");
			if (!source_text) {
				fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
				return EXIT_FAILURE;
			}
			// printf("Input mode: File\n");
			break;
		default:
			fprintf(stderr, "Unknown input mode.\n");
			echo_usage(argv[0]);
			return EXIT_FAILURE;
	}

	char *buffer = copy_to_buffer(source_text);
	if (!buffer) {
		if (source_text != stdin) fclose(source_text);
		return EXIT_FAILURE;
	}
	if (source_text != stdin) fclose(source_text);

	FILE *user_input_stream = fopen(USER_INPUT_DEVICE, "r");
	if (!user_input_stream) {
		fprintf(stderr, "Error: Could not open terminal for user input.\n");
		free(buffer);
		return EXIT_FAILURE;
	}

	long total_lines = count_lines(buffer);
	if (buffer[0] == '\0') {
		fprintf(stderr, "Error: Input is empty.\n");
		free(buffer);
		fclose(user_input_stream);
		return EXIT_FAILURE;
	}
	long starting_line = 1;
	unsigned long offset = 0;
	if (argc == 3) {
		starting_line = validate_line_number(argv[2], total_lines);
		set_starting_offset(buffer, &offset, starting_line);
	}
	const char *source_label = (mode == INPUT_MODE_PIPE) ? "stdin" : argv[1];
	printf("Reading from '%s', starting from line %ld of %ld.\n", source_label, starting_line, total_lines);
	long line = starting_line;
	char *user_input = NULL;
	size_t buffer_length = strlen(buffer);
	
	while (line < total_lines + 1) {
		// Save offset before printing
		unsigned long line_offset = offset;
		
		int last_line = print_next_line(buffer, &offset, buffer_length);
		
		size_t printed_line_len = 0;
		const char *line_start = &buffer[line_offset];
		const char *line_end = strchr(line_start, '\n');
		if (line_end) {
			printed_line_len = line_end - line_start;
		} else {
			printed_line_len = strlen(line_start); // fallback: no newline
		}
		
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
		
		size_t input_capacity = printed_line_len + 2; // +1 for newline, +1 for null terminator
		user_input = malloc(input_capacity);
		if (!user_input) {
			fprintf(stderr, "Error: Failed to allocate memory for user input.\n");
			free(buffer);
			fclose(user_input_stream);
			return EXIT_FAILURE;
		}

		if (fgets(user_input, input_capacity, user_input_stream)) {
			user_input[strcspn(user_input, "\n")] = '\0'; // strip newline if present
		} else {
			// End-of-input or error
			switch (mode) {
				case INPUT_MODE_PIPE:
					printf("Program ended.  Example resume command:\n");
					printf("cat original.txt | ");
					break;
				case INPUT_MODE_FILE:
					printf("Program ended on line %ld of %ld\n", line, total_lines);
					printf("To continue from this point next time use the command:\n");
					break;
				default:
					break;
			}
			printf("%s %s %ld\n", argv[0], argv[1], line);
			free(user_input);
			free(buffer);
			fclose(user_input_stream);
			return EXIT_FAILURE;
		}

		printf("\n");
		free(user_input);
		user_input = NULL; // reset for next loop
		line++;
	}

	free(buffer);
	fclose(user_input_stream);
	return 0;
}
