#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 1024

// Utility: Check if a string is numeric
int is_numeric(const char *str) {
    for (int i = 0; str[i]; ++i) {
        if (!isdigit((unsigned char)str[i])) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s filename.txt [start_line]\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    int total_lines = 0;
    char temp[MAX_LINE_LEN];

    // First pass: count total lines
    while (fgets(temp, sizeof(temp), file)) {
        total_lines++;
    }

    // Optional: starting line number
    int start_line = 1;
    if (argc == 3) {
        if (!is_numeric(argv[2])) {
            fprintf(stderr, "Invalid start_line: not a number\n");
            fclose(file);
            return 1;
        }

        start_line = atoi(argv[2]);
        if (start_line < 1 || start_line > total_lines) {
            fprintf(stderr, "Start line out of range (1 to %d)\n", total_lines);
            fclose(file);
            return 1;
        }
    }

    // Rewind file and skip to start_line
    rewind(file);
    int file_line_number = 1;
    while (file_line_number < start_line && fgets(temp, sizeof(temp), file)) {
        file_line_number++;
    }

    char line[MAX_LINE_LEN];
    char input[MAX_LINE_LEN];
    int current_line = start_line;

    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        line[strcspn(line, "\n")] = '\0';

        // Skip blank lines
        if (line[0] == '\0') {
            current_line++;
            continue;
        }

        printf("\n%s\n", line);

        if (!fgets(input, sizeof(input), stdin)) {
            if (feof(stdin)) {
                printf("\nEOF received. You left off at line %d.\n", current_line);
                printf("\nTo continue from this point, use the command:\n%s %s %d\n", 
                    argv[0], argv[1], current_line);
            } else {
                printf("\nInput error.\n");
            }
            break;
        }

        // Remove newline from input
        input[strcspn(input, "\n")] = '\0';

        current_line++;
    }

    fclose(file);
    printf("%s", "\n");
    return 0;
}
