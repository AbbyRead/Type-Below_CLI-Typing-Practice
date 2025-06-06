#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 1024

// Utility: Check if a string is numeric
int is_numeric(const char *str) {
	for (int i = 0; str[i]; ++i) {
		if (!isdigit((unsigned char)str[i])) return 0;
	}
	return 1;
}

int main(int argc, char *argv[]) {
	FILE *file = NULL;
	int using_stdin = 0;

	// Check for filename argument
	if (argc < 2 || (argc == 2 && !is_numeric(argv[1]) && strcmp(argv[1], "-") == 0)) {
		// No filename or "-" given: use stdin
		file = stdin;
		using_stdin = 1;
	} else {
		file = fopen(argv[1], "r");
		if (!file) {
			fprintf(stderr, "Error opening file '%s', falling back to stdin.\n", argv[1]);
			file = stdin;
			using_stdin = 1;
		}
	}

	int start_line = 1;
	if (argc == 3) {
		if (!is_numeric(argv[2])) {
			fprintf(stderr, "Invalid start_line: not a number\n");
			if (!using_stdin) fclose(file);
			return 1;
		}
		start_line = atoi(argv[2]);
		if (start_line < 1) {
			fprintf(stderr, "Start line must be >= 1\n");
			if (!using_stdin) fclose(file);
			return 1;
		}
	}

	char line[MAX_LINE_LEN];
	char input[MAX_LINE_LEN];
	int current_line = 1;

	// Skip lines if start_line > 1
	while (current_line < start_line && fgets(line, sizeof(line), file)) {
		current_line++;
	}

	while (fgets(line, sizeof(line), file)) {
		// Remove trailing newline
		line[strcspn(line, "\n")] = '\0';

		// Skip blank lines
		if (line[0] == '\0') {
			current_line++;
			continue;
		}

		printf("\n%s\n", line);

		if (!fgets(input, sizeof(input), stdin)) {
			if (feof(stdin)) {
				printf("\nEOF received. You left off at line %d.\n", current_line);
				if (!using_stdin) {
					printf("\nTo continue from this point, use the command:\n%s %s %d\n",
						argv[0], argv[1], current_line);
				}
			} else {
				printf("\nInput error.\n");
			}
			break;
		}

		// Remove newline from input
		input[strcspn(input, "\n")] = '\0';

		current_line++;
	}

	if (!using_stdin) fclose(file);
	printf("%s", "\n");
	return 0;
}
