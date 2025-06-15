#include <stdio.h>
#include <stdlib.h>

#include "platform.h"

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	char *buffer = platform_get_clipboard();
	if (!buffer) return 1;
	printf("%s\n", buffer);
	free(buffer);
	return 0;
}