#ifndef PLATFORM_H
#define PLATFORM_H

void platform_init(void);
void platform_cleanup(void);

// Clipboard — returns malloc'd string or NULL if unavailable
char *platform_get_clipboard(void);

// Terminal size
int platform_get_terminal_height(void);
int platform_get_terminal_width(void);

// Cursor control
void platform_set_cursor_pos(int row, int col);
void platform_hide_cursor(void);
void platform_show_cursor(void);

// Sleep utility (e.g. blinking cursor)
void platform_sleep_ms(int milliseconds);

#endif
