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

int get_terminal_height(void) {
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
