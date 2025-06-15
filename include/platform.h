#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32

#include <io.h>
#include <windows.h>
#define isatty _isatty
#define fileno _fileno
#define USER_INPUT_DEVICE "CON"
static inline int get_terminal_height() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
		return 24;
	}
	int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return (height > 0) ? height : 24;
}
static inline void move_cursor_up(int lines) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    pos.X = 0;
    pos.Y = csbi.dwCursorPosition.Y - lines;
    SetConsoleCursorPosition(hOut, pos);
}

#else // POSIX
#include <sys/ioctl.h>
#include <unistd.h>
#define USER_INPUT_DEVICE "/dev/tty"
static inline int get_terminal_height() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int height = w.ws_row;
	if (height <= 0) height = 24;
    return height;
}
static inline void move_cursor_up(int lines) {
    printf("\033[%dA", lines);
}

#endif // Windows vs. POSIX

#endif // PLATFORM_H
