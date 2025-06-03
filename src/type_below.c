#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_CAPACITY 8192
#define MAX_LINE_LEN 1024

// Read entire stdin into dynamically allocated buffer
char *read_all_stdin(void) {
    size_t cap = INITIAL_CAPACITY;
    size_t len = 0;
    char *buffer = malloc(cap);
    if (!buffer) return NULL;

    int c;
    while ((c = getchar()) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char *newbuf = realloc(buffer, cap);
            if (!newbuf) {
                free(buffer);
                return NULL;
            }
            buffer = newbuf;
        }
        buffer[len++] = c;
    }

    buffer[len] = '\0';  // Null-terminate
    return buffer;
}

// Count total lines in the buffer
int count_lines(const char *text) {
    int lines = 0;
    for (const char *p = text; *p; ++p) {
        if (*p == '\n') lines++;
    }
    return lines;
}

// Split buffer into lines (modifies buffer in-place)
char **split_lines(char *text, int *out_count) {
    int lines = count_lines(text);
    char **line_ptrs = malloc(sizeof(char*) * lines);
    int count = 0;

    char *line = strtok(text, "\n");
    while (line) {
        line_ptrs[count++] = line;
        line = strtok(NULL, "\n");
    }

    *out_count = count;
    return line_ptrs;
}

// Check if a string is numeric
int is_numeric(const char *str) {
    for (int i = 0; str[i]; ++i)
        if (!isdigit((unsigned char)str[i])) return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    int start_line = 1;

    if (argc > 2 || (argc == 2 && !is_numeric(argv[1]) && strcmp(argv[1], "-") != 0)) {
        fprintf(stderr, "Usage: %s [start_line]\n", argv[0]);
        return 1;
    }

    if (argc == 2 && is_numeric(argv[1])) {
        start_line = atoi(argv[1]);
        if (start_line < 1) start_line = 1;
    }

    char *text = read_all_stdin();
    if (!text) {
        fprintf(stderr, "Failed to read input.\n");
        return 1;
    }

    int total_lines = 0;
    char **lines = split_lines(text, &total_lines);
    if (!lines) {
        free(text);
        fprintf(stderr, "Failed to parse lines.\n");
        return 1;
    }

    if (start_line > total_lines) {
        fprintf(stderr, "Start line out of range (1 to %d)\n", total_lines);
        free(lines);
        free(text);
        return 1;
    }

    FILE *tty = fopen("/dev/tty", "r");
    if (!tty) {
        perror("Cannot open /dev/tty for user input");
        free(lines);
        free(text);
        return 1;
    }

    char input[MAX_LINE_LEN];
    for (int i = start_line - 1; i < total_lines; ++i) {
        if (lines[i][0] == '\0') continue;

        printf("\n%s\n", lines[i]);

        if (!fgets(input, sizeof(input), tty)) {
            printf("\nEOF or input error. You left off at line %d.\n", i + 1);
            printf("To continue: pbpaste | %s %d\n", argv[0], i + 1);
            break;
        }
        input[strcspn(input, "\n")] = '\0';  // Trim newline
    }

    fclose(tty);
    free(lines);
    free(text);
    return 0;
}
