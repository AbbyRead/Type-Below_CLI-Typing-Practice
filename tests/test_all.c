#include <stdio.h>
extern int test_platform_get_clipboard(void);

int main(void) {
	printf("test_platform_get_clipboard() returned %d.\n", test_platform_get_clipboard());
}
