#include "core.h"
#include "platform.h"

void echo_usage(const char *prog_name) {
	fprintf(stderr,
		"Usage:\n"
		"  %s [options] filename    # Source text from an existing file\n"
		"  %s [options] -           # Source text from stdin via piping/redirection\n"
		"  %s [options]             # Source text from the OS copy/paste clipboard\n\n"
		"Options:\n"
		"  -s <line_number>         Start from the specified line (negative = offset from end)\n"
		"  -h, --help               Show this help message and exit\n"
		"  -v, --version            Show program version and exit\n\n"
		"Examples:\n"
		"  %s -s 10 myfile.txt      # Start at line 10 from file\n"
		"  %s -s -3 -               # Start 3 lines from end, read from stdin\n"
		"  %s                       # Read from clipboard, start at line 1\n",
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
