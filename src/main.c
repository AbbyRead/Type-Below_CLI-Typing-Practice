#include "core.h"
#include "ingest.h"
#include "platform.h"
#include "version.h"

int main(int argc, char *argv[]) {
	precheck_arguments(argc, argv);

	const char *file_arg = NULL;
	const char *start_line_arg = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-s") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Error: Missing argument for -s\n");
				echo_usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			start_line_arg = argv[++i];
		} else if (argv[i][0] != '-') {
			if (file_arg) {
				fprintf(stderr, "Error: Multiple input sources specified.\n");
				echo_usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			file_arg = argv[i];
		}
	}

	enum InputMode mode = determine_input_mode(file_arg);
	FILE *source_text = NULL;

	switch (mode) {
		case INPUT_MODE_PIPE:
			#ifndef _WIN32
			if (isatty(fileno(stdin))) {
				fprintf(stderr, "Error: No piped-in text detected on stdin.\n");
				return EXIT_FAILURE;
			}
			#endif
			source_text = stdin;
			break;
		case INPUT_MODE_FILE:
			source_text = fopen(file_arg, "r");
			if (!source_text) {
				fprintf(stderr, "Error opening file '%s': %s\n", file_arg, strerror(errno));
				return EXIT_FAILURE;
			}
			break;
		case INPUT_MODE_CLIPBOARD:
			// will call special function below
			break;
		default:
			fprintf(stderr, "Unknown input mode.\n");
			echo_usage(argv[0]);
			return EXIT_FAILURE;
	}

	char *buffer = NULL;
	if (mode == INPUT_MODE_CLIPBOARD) {
		buffer = copy_from_clipboard();
	} else {
		buffer = copy_to_buffer(source_text);
		if (!buffer) {
			if (source_text != stdin) fclose(source_text);
			return EXIT_FAILURE;
		}
		if (source_text != stdin) fclose(source_text);
	}


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

	if (start_line_arg) {
		starting_line = validate_line_number(start_line_arg, total_lines);
		set_starting_offset(buffer, &offset, starting_line);
	}

	const char *source_label =
		mode == INPUT_MODE_CLIPBOARD ? "clipboard" :
		mode == INPUT_MODE_PIPE ? "stdin" : file_arg;
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
		printed_line_len = line_end ? (line_end - line_start) : strlen(line_start);

		if (last_line) printf("\n");

		int height = get_terminal_height();
		int pad_lines = (int)(height * 0.4f);
		if (pad_lines < 1) pad_lines = 1;
		for (int i = 0; i < pad_lines; i++) printf("\n");
		move_cursor_up(pad_lines);
		fflush(stdout);

		size_t input_capacity = printed_line_len + 64; // Allow extra typing room
		user_input = malloc(input_capacity);
		if (!user_input) {
			fprintf(stderr, "Error: Failed to allocate memory for user input.\n");
			goto cleanup;
		}

		if (!fgets(user_input, input_capacity, user_input_stream)) {
			printf("\n"); // Extra newline for cleanliness
			switch (mode) {				
				case INPUT_MODE_CLIPBOARD:
					printf("Program ended on line %ld of %ld.\n", line, total_lines);
					printf("To resume from the same clipboard content, use:\n");
					printf("%s -s %ld\n", argv[0], line);
					break;
				case INPUT_MODE_PIPE:
					printf("Program ended on line %ld of %ld.\n", line, total_lines);
					printf("To resume from the same piped or redirected content, use:\n");
					printf("%s -s %ld -\n", argv[0], line);
					break;
				case INPUT_MODE_FILE:
					printf("Program ended on line %ld of %ld.\n", line, total_lines);
					printf("To continue from this point next time use the command:\n");
					printf("%s -s %ld %s\n", argv[0], line, file_arg);
					break;
				default: break;
			}
			goto cleanup;
		}

		user_input[strcspn(user_input, "\n")] = '\0'; // Strip newline
		printf("\n");

		free(user_input);
		user_input = NULL;
		line++;
	}

	cleanup:
		if (user_input) free(user_input);
		if (buffer) free(buffer);
		if (user_input_stream) fclose(user_input_stream);
		return 0;
}
