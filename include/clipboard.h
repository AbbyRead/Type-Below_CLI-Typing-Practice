#ifndef CLIPBOARD_H
#define CLIPBOARD_H

// Reads UTF-8 text from the clipboard.
// Returns a malloc'd null-terminated string on success.
// The caller must free() the result.
// Returns NULL if clipboard is empty or failed to initialize.

char *read_clipboard(void);

#endif // CLIPBOARD_READER_H
