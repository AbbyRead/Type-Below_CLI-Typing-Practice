#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename.txt\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE_LEN];
    char input[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline if present
        line[strcspn(line, "\n")] = '\0';

        // Skip blank lines
        if (line[0] == '\0') {
            continue;
        }

        printf("\n%s\n", line);

        if (!fgets(input, sizeof(input), stdin)) {
            printf("\nInput error.\n");
            break;
        }

        // Remove newline from input
        input[strcspn(input, "\n")] = '\0';

        // You could add a comparison check here if desired
    }

    fclose(file);
    printf("\nDone.\n");
    return 0;
}
