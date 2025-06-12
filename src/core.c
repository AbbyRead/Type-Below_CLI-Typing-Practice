#include "core.h"
#include "platform.h"

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

int print_next_line(const char *buffer, unsigned long *offset, size_t buffer_length) {
	if (*offset >= buffer_length) return 1;

	const char *line_start = &buffer[*offset];
	const char *newline_pos = strchr(line_start, '\n');
	size_t line_length = newline_pos ? (newline_pos - line_start + 1) : (buffer_length - *offset);

	fwrite(line_start, 1, line_length, stdout);
	*offset += line_length;
	return (*offset >= buffer_length);
}
