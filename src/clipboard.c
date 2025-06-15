#include <stdio.h>
#include <stdlib.h>
#include <libclipboard.h>
#include "clipboard.h"

char *read_clipboard(void) {
    clipboard_c *cb = clipboard_new(NULL);
    if (!cb) {
        fprintf(stderr, "Failed to initialize clipboard\n");
        return NULL;
    }

    char *text = clipboard_text(cb);
    clipboard_free(cb);
    return text;
}
