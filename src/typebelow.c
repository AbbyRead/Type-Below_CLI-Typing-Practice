#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void echo_usage(const char *prog_name) {
	fprintf(stderr, "Usage: %s <filename> [starting_line]\n", prog_name);
}

long validate_line_number(const char *line_number_str) {
	char *end;
	errno = 0; // Reset errno before strtoul
	long line_number = strtoul(line_number_str, &end, 10);
	if (errno != 0 || *end != '\0') {
		line_number = 0; // error value
	}
	return line_number;
}

// Main function to read a file or stdin and print lines one by one
int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Expected a filename as input.\n");
		echo_usage(*argv);
		exit(EXIT_FAILURE);
	}
	if (argc > 3) {
		fprintf(stderr, "Too many arguments provided.\n");
		echo_usage(*argv);
		exit(EXIT_FAILURE);
	}
	if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
		echo_usage(*argv);
		exit(EXIT_SUCCESS);
	}

	FILE *file = fopen(argv[1], "r");
	if (!file) {
		fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	long starting_line = 1; // Default starting line is 1
	long lines = 0;
	if (argc == 3) { // If a starting line is provided as the second argument
		starting_line = validate_line_number(argv[2]);
		if (starting_line == 0) {
			fprintf(stderr, "Invalid starting line number: '%s'. It must be an integer.\n", argv[2]);
			echo_usage(*argv);
			exit(EXIT_FAILURE);
		}
		if (starting_line < 0) {
			long c;
			long last = '\n';
			long lines = 0;
			// Count lines in the file to know where to backtrack to from the end
			fseek(file, 0, SEEK_END); // Move to the end of the file
			while ((c = fgetc(file)) != EOF) {
				if (c == '\n') lines++;
				last = c;
			}
			if (starting_line > lines) {
				fprintf(stderr, "Starting line number %lu exceeds total lines %lu in the file.\n", starting_line, lines);
				fclose(file);
				exit(EXIT_FAILURE);
			}
			fseek(file, 0, SEEK_SET); // Reset to the beginning of the file
			lines = 0;
			while ((c = fgetc(file)) != EOF) {
				if (c == '\n') lines++;
				if (lines == (starting_line + 1)) break; // Stop when we reach the desired line
			}
			if (c == EOF) {
				fprintf(stderr, "Reached end of file before finding line %lu.\n", starting_line);
				fclose(file);
				exit(EXIT_FAILURE);
			}
			printf("%ld", lines);
			if (last != '\n') lines++;  // Count the last line if it lacks newline
			printf("Starting from line: %ld\n", lines + starting_line);
			starting_line = lines + starting_line; // Convert negative to positive offset
		}
		printf("Total lines in file: %lu\n", lines);
		if (lines == 0) {
			fprintf(stderr, "The file is empty.\n");
			fclose(file);
			exit(EXIT_FAILURE);
		}
	}

	printf("Reading from file: %s, starting from line: %ld\n", argv[1], starting_line);

	return 0;
}
