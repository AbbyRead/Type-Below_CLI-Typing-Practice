#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_MATCH 0
#define CHUNK_SIZE 4096

// Operating mode enums: piped input or file input
enum InputMode {
    INPUT_MODE_FILE,
    INPUT_MODE_PIPE
};

void echo_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <filename> [starting_line]\n", prog_name);
}

void precheck_arguments(int argc, char *argv[]) {
    if (argc < 2) { // At least one argument is required (the filename)
        fprintf(stderr, "Expected a filename or '-' to specify source text.\n");
        echo_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc > 3) { // Allow at most two arguments: filename and optional starting line
        fprintf(stderr, "Too many arguments provided.\n");
        echo_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == STRING_MATCH) {
        echo_usage(argv[0]);
        exit(EXIT_SUCCESS);
    }
}

enum InputMode determine_input_mode(const char *file_arg) {
    if (strcmp(file_arg, "-") == STRING_MATCH) {
        return INPUT_MODE_PIPE;
    } else {
        return INPUT_MODE_FILE;
    }
}

char *copy_to_buffer(FILE *stream) {
    if (!stream) {
        perror("Invalid stream");
        return NULL;
    }

    size_t bytes_available = CHUNK_SIZE;
    size_t byte_count = 0;
    char *buffer = malloc(bytes_available);
    if (!buffer) {
        perror("Failed to allocate memory");
        return NULL;
    }

    while (1) {
        int c = getc(stream);
        if (c == EOF || ferror(stream)) break;
        buffer[byte_count++] = (char)c;
        if (byte_count == bytes_available) {
            bytes_available += CHUNK_SIZE;
            char *new_buffer = realloc(buffer, bytes_available);
            if (!new_buffer) {
                perror("Failed to reallocate memory");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
    }
    buffer[byte_count] = '\0';

    char *final_buffer = realloc(buffer, byte_count + 1);
    if (!final_buffer) {
        perror("Failed to reallocate final memory");
        free(buffer);
        return NULL;
    }

    return final_buffer;
}

long count_lines(const char *buffer) {
    long lines = 1;
    while (*buffer != '\0') {
        if (*buffer++ == '\n') lines++;
    }
    return lines;
}

long validate_line_number(const char *buffer, const char *line_number_str, const long total_lines) {
    errno = 0;
    char *endptr;
    long line_number = strtol(line_number_str, &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Invalid starting line number: %s\n", line_number_str);
        exit(EXIT_FAILURE);
    }

    if (line_number > total_lines) {
        printf("Starting line specified: %ld is greater than number of lines available: %ld.\n",
               line_number, total_lines);
        exit(EXIT_FAILURE);
    }
    if (line_number < 0) {
        line_number += total_lines;
        if (line_number < 1) {
            printf("%s\n", "Starting line offset from end is greater than the total number of lines.");
            printf("Total: %ld\tSpecified: %s\nWhich would evaluate as line %ld.\n",
                   total_lines, line_number_str, line_number);
            exit(EXIT_FAILURE);
        }
    }
    return line_number;
}

int print_next_line(const char *buffer, unsigned long *offset) {
    if (*offset >= strlen(buffer)) return 1;

    const char *line_start = &buffer[*offset];
    const char *newline_pos = strchr(line_start, '\n');
    size_t line_length = newline_pos ? (newline_pos - line_start + 1) : strlen(line_start);

    fwrite(line_start, 1, line_length, stdout);
    *offset += line_length;
    return (*offset >= strlen(buffer));
}

int main(int argc, char *argv[]) {
    precheck_arguments(argc, argv);

    enum InputMode mode;
    FILE *source_text = NULL;

    mode = determine_input_mode(argv[1]);

    switch (mode) {
        case INPUT_MODE_PIPE:
            source_text = stdin;
            printf("Input mode: Piped input (stdin)\n");
            break;
        case INPUT_MODE_FILE:
            source_text = fopen(argv[1], "r");
            if (!source_text) {
                fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
                return EXIT_FAILURE;
            }
            printf("Input mode: File\n");
            break;
        default:
            fprintf(stderr, "Unknown input mode.\n");
            echo_usage(argv[0]);
            return EXIT_FAILURE;
    }

    char *buffer = copy_to_buffer(source_text);
    if (!buffer) {
        fclose(source_text);
        return EXIT_FAILURE;
    }
    fclose(source_text);

	long total_lines = count_lines(buffer);
    long starting_line = 1;
    if (argc == 3) { // If a starting line is provided as the second argument
        starting_line = validate_line_number(buffer, argv[2], total_lines);
    }

    printf("Reading from '%s', starting from line %ld.\n", argv[1], starting_line);

    unsigned long offset = 0;
    for (long i = 1; i < starting_line; ++i) {
        if (print_next_line(buffer, &offset)) break;
    }

    while (!print_next_line(buffer, &offset)) {
        char user_input[1024];
        if (fgets(user_input, sizeof(user_input), stdin)) {
            // Remove newline if present
            user_input[strcspn(user_input, "\n")] = '\0';
        } else {
            perror("Program ended by user.");
            free(buffer);
            return EXIT_FAILURE;
        }
        printf("\n");
    }

    free(buffer);
    return 0;
}
