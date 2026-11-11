#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int append_file(const char *filename)
{
    char line[4096];
    FILE *file = fopen(filename, "a");

    if (file == NULL) {
        perror("fopen");
        return 1;
    }
    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (fputs(line, file) == EOF) {
            perror("fputs");
            fclose(file);
            return 1;
        }
    }
    if (ferror(stdin)) {
        perror("fgets");
        fclose(file);
        return 1;
    }
    if (fclose(file) == EOF) {
        perror("fclose");
        return 1;
    }
    return 0;
}

static int rewrite_file(const char *filename, size_t target, int insert)
{
    char line[4096], inserted_line[4096], tmp[4096];
    FILE *in = fopen(filename, "r");
    FILE *out;
    size_t line_no = 1;
    int found = 0, error = 0, last_newline = 1;
    int written;

    if (in == NULL) {
        perror("fopen");
        return 1;
    }
    written = snprintf(tmp, sizeof(tmp), "%s.tmp", filename);
    if (written < 0 || (size_t)written >= sizeof(tmp)) {
        fprintf(stderr, "Ime datoteke je predolgo\n");
        fclose(in);
        return 1;
    }
    if (insert) {
        size_t length;

        if (fgets(inserted_line, sizeof(inserted_line), stdin) == NULL) {
            fprintf(stderr, "Vrstice za vstavljanje ni bilo mogoče prebrati\n");
            fclose(in);
            return 1;
        }
        length = strlen(inserted_line);
        if (strchr(inserted_line, '\n') == NULL) {
            if (length + 1 >= sizeof(inserted_line)) {
                fprintf(stderr, "Vrstica je predolga\n");
                fclose(in);
                return 1;
            }
            inserted_line[length] = '\n';
            inserted_line[length + 1] = '\0';
        }
    }
    out = fopen(tmp, "w");
    if (out == NULL) {
        perror("fopen");
        fclose(in);
        return 1;
    }

    while (fgets(line, sizeof(line), in) != NULL) {
        last_newline = strchr(line, '\n') != NULL;
        if (insert && !found && line_no == target) {
            if (fputs(inserted_line, out) == EOF) {
                perror("fputs");
                error = 1;
                break;
            }
            found = 1;
        }
        if (!insert && line_no == target)
            found = 1;
        else if (fputs(line, out) == EOF) {
            perror("fputs");
            error = 1;
            break;
        }
        if (last_newline)
            line_no++;
    }
    if (ferror(in)) {
        perror("fgets");
        error = 1;
    }
    if (!error && !last_newline)
        line_no++;
    if (!error && insert && !found && line_no == target) {
        if (!last_newline && fputc('\n', out) == EOF) {
            perror("fputc");
            error = 1;
        } else if (fputs(inserted_line, out) == EOF) {
            perror("fputs");
            error = 1;
        } else {
            found = 1;
        }
    }
    if (fclose(in) == EOF) {
        perror("fclose");
        error = 1;
    }
    if (fclose(out) == EOF) {
        perror("fclose");
        error = 1;
    }
    if (!found && !error) {
        fprintf(stderr, "Vrstica %zu ne obstaja\n", target);
        error = 1;
    }
    if (error) {
        unlink(tmp);
        return 1;
    }
    if (unlink(filename) == -1) {
        perror("unlink");
        unlink(tmp);
        return 1;
    }
    if (rename(tmp, filename) == -1) {
        perror("rename");
        unlink(tmp);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    char *end;
    unsigned long long parsed;

    if (argc == 3 && strcmp(argv[1], "-a") == 0)
        return append_file(argv[2]);

    if (argc == 4 &&
        (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "-i") == 0)) {
        errno = 0;
        parsed = strtoull(argv[2], &end, 10);
        if (argv[2][0] == '-' || end == argv[2] || *end != '\0' ||
            errno != 0 || parsed == 0 || parsed > SIZE_MAX) {
            fprintf(stderr, "Neveljavna številka vrstice\n");
            return 1;
        }
        return rewrite_file(argv[3], (size_t)parsed, strcmp(argv[1], "-i") == 0);
    }

    fprintf(stderr, "Uporaba: %s -a <datoteka> | -d <n> <datoteka> | -i <n> <datoteka>\n",
            argv[0]);
    return 1;
}