#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"

char *platform_get_clipboard(void) {
	if (!OpenClipboard(NULL)) {
		return NULL;
	}

	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
	if (!hData) {
		CloseClipboard();
		return NULL;
	}

	LPCWSTR wtext = (LPCWSTR)GlobalLock(hData);
	if (!wtext) {
		CloseClipboard();
		return NULL;
	}

	// Convert from wide char to UTF-8
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wtext, -1, NULL, 0, NULL, NULL);
	if (size_needed <= 0) {
		GlobalUnlock(hData);
		CloseClipboard();
		return NULL;
	}

	char *buffer = malloc(size_needed);
	if (!buffer) {
		GlobalUnlock(hData);
		CloseClipboard();
		return NULL;
	}

	WideCharToMultiByte(CP_UTF8, 0, wtext, -1, buffer, size_needed, NULL, NULL);

	GlobalUnlock(hData);
	CloseClipboard();

	return buffer;
}
