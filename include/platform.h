#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define isatty _isatty
#define fileno _fileno
#define USER_INPUT_DEVICE "CON"
#else
#include <sys/ioctl.h>
#include <unistd.h>
#define USER_INPUT_DEVICE "/dev/tty"
#endif

#endif
