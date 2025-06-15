#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "platform.h"

char *platform_get_clipboard(void) {
	FILE *pipe = popen("pbpaste", "r");
	if (!pipe) return NULL;

	size_t capacity = 1024;
	size_t length = 0;
	char *buffer = malloc(capacity);
	if (!buffer) {
		pclose(pipe);
		return NULL;
	}

	int c;
	while ((c = fgetc(pipe)) != EOF) {
		if (length + 1 >= capacity) {
			capacity *= 2;
			char *newbuf = realloc(buffer, capacity);
			if (!newbuf) {
				free(buffer);
				pclose(pipe);
				return NULL;
			}
			buffer = newbuf;
		}
		buffer[length++] = (char)c;
	}
	pclose(pipe);

	if (length == 0) {
		free(buffer);
		return NULL;
	}

	buffer[length] = '\0';
	return buffer;
}
