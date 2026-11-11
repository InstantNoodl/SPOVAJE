#define _POSIX_C_SOURCE 199309L
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int parse_size(const char *text, size_t *value)
{
    char *end;
    unsigned long long parsed;

    if (*text == '\0' || *text == '-')
        return -1;
    errno = 0;
    parsed = strtoull(text, &end, 10);
    if (errno != 0 || *end != '\0' || parsed > SIZE_MAX)
        return -1;
    *value = (size_t)parsed;
    return 0;
}

int main(int argc, char **argv)
{
    char buffer[4096];
    size_t buffer_size, count;
    FILE *file;
    struct timespec start, end;

    if (argc != 3 || parse_size(argv[1], &buffer_size) != 0 ||
        parse_size(argv[2], &count) != 0 ||
        buffer_size == 0 || buffer_size > sizeof(buffer)) {
        fprintf(stderr, "Uporaba: %s <B: 1-4096> <N>\n", argv[0]);
        return 1;
    }

    file = fopen("izhod.txt", "w");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }
    if (setvbuf(file, buffer, _IOFBF, buffer_size) != 0) {
        fprintf(stderr, "setvbuf ni uspel\n");
        fclose(file);
        return 1;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        perror("clock_gettime");
        fclose(file);
        return 1;
    }
    for (size_t i = 0; i < count; i++) {
        if (fprintf(file, "Program zapisuje stavek v datoteko.\n") < 0) {
            perror("fprintf");
            fclose(file);
            return 1;
        }
    }
    if (fflush(file) == EOF) {
        perror("fflush");
        fclose(file);
        return 1;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
        perror("clock_gettime");
        fclose(file);
        return 1;
    }
    if (fclose(file) == EOF) {
        perror("fclose");
        return 1;
    }

    printf("B = %zu, N = %zu, cas = %.6f s\n", buffer_size, count,
           (double)(end.tv_sec - start.tv_sec) +
           (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0);
    return 0;
}