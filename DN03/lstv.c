#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct entry {
    char *name;
    time_t modified;
};

static int reversed;

static int compare_entries(const void *a, const void *b)
{
    const struct entry *x = a;
    const struct entry *y = b;

    if (x->modified == y->modified)
        return 0;
    return (x->modified < y->modified) != reversed ? 1 : -1;
}

int main(int argc, char **argv)
{
    DIR *dir;
    struct dirent *item;
    struct entry *entries;
    size_t count = 0, i = 0;

    if (argc > 2 || (argc == 2 && strcmp(argv[1], "-r") != 0)) {
        fprintf(stderr, "Usage: %s [-r]\n", argv[0]);
        return 1;
    }
    reversed = argc == 2;

    dir = opendir(".");
    if (dir == NULL) {
        perror(".");
        return 1;
    }

    while ((item = readdir(dir)) != NULL) {
        if (strcmp(item->d_name, ".") != 0 &&
            strcmp(item->d_name, "..") != 0)
            count++;
    }
    rewinddir(dir);

    entries = malloc(count * sizeof(*entries));
    if (entries == NULL && count != 0) {
        perror("malloc");
        closedir(dir);
        return 1;
    }

    while ((item = readdir(dir)) != NULL) {
        struct stat info;

        if (strcmp(item->d_name, ".") == 0 ||
            strcmp(item->d_name, "..") == 0)
            continue;
        if (stat(item->d_name, &info) == -1) {
            perror(item->d_name);
            while (i > 0)
                free(entries[--i].name);
            free(entries);
            closedir(dir);
            return 1;
        }
        entries[i].name = malloc(strlen(item->d_name) + 1);
        if (entries[i].name == NULL) {
            perror("malloc");
            while (i > 0)
                free(entries[--i].name);
            free(entries);
            closedir(dir);
            return 1;
        }
        strcpy(entries[i].name, item->d_name);
        entries[i].modified = info.st_mtime;
        i++;
    }
    closedir(dir);

    qsort(entries, i, sizeof(*entries), compare_entries);
    for (size_t j = 0; j < i; j++) {
        printf("%s ", entries[j].name);
        free(entries[j].name);
    }
    printf("\n");
    free(entries);
    return 0;
}