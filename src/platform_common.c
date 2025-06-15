#include "platform.h"

// Default stub: no clipboard support, return NULL
char *platform_get_clipboard(void) {
	return NULL;
}
