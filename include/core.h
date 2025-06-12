#ifndef CORE_H
#define CORE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void echo_usage(const char *prog_name);
int print_next_line(const char *buffer, unsigned long *offset, size_t buffer_length);

#endif
