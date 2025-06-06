#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#define MAX_LINE_LEN 1024

int main(int argc, char *argv[]) {
    FILE *file = NULL;

    if (argc == 2) {
        file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
            return 1;
        }
    } else if (!isatty(fileno(stdin))) {
        // Data is being piped in
        file = stdin;
    } else {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        fprintf(stderr, "       or pipe data into the program.\n");
        return 1;
    }

    char line[MAX_LINE_LEN];
    char input[MAX_LINE_LEN];
    int current_line = 1;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0') {
            current_line++;
            continue;
        }

        printf("\n%s\n", line);

        if (!fgets(input, sizeof(input), stdin)) {
            printf("\nInput error or EOF. Exiting at line %d.\n", current_line);
            break;
        }

        input[strcspn(input, "\n")] = '\0';
        current_line++;
    }

    if (file != stdin) {
        fclose(file);
    }

    printf("\n");
    return 0;
}
