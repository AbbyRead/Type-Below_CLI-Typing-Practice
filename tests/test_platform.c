#include <stdio.h>
#include <stdlib.h>
#include "../include/platform.h"

int test_platform_get_clipboard(void) {
    char *buffer = platform_get_clipboard();
    if (!buffer) {
        printf("FAIL: Clipboard empty or inaccessible\n");
        return 1;
    }
    printf("PASS: Clipboard contents = \"%s\"\n", buffer);
    free(buffer);
    return 0;
}
