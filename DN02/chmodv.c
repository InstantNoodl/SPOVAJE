#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void format_permissions(mode_t mode, char permissions[10])
{
    permissions[0] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (mode & S_ISUID)
        ? ((mode & S_IXUSR) ? 's' : 'S')
        : ((mode & S_IXUSR) ? 'x' : '-');
    permissions[3] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (mode & S_ISGID)
        ? ((mode & S_IXGRP) ? 's' : 'S')
        : ((mode & S_IXGRP) ? 'x' : '-');
    permissions[6] = (mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (mode & S_ISVTX)
        ? ((mode & S_IXOTH) ? 't' : 'T')
        : ((mode & S_IXOTH) ? 'x' : '-');
    permissions[9] = '\0';
}

static int parse_octal_mode(const char *text, mode_t *mode)
{
    const char *digit;
    char *end;
    unsigned long value;

    if (*text == '\0')
        return 0;

    for (digit = text; *digit != '\0'; digit++) {
        if (*digit < '0' || *digit > '7')
            return 0;
    }

    errno = 0;
    value = strtoul(text, &end, 8);
    if (errno != 0 || *end != '\0' || value > 07777)
        return 0;

    *mode = (mode_t)value;
    return 1;
}

int main(int argc, char *argv[])
{
    struct stat file_info;
    mode_t new_mode;
    char permissions[10];

    if (argc != 3) {
        fprintf(stderr, "Uporaba: %s <dovoljenja> <datoteka>\n", argv[0]);
        return 1;
    }

    if (stat(argv[2], &file_info) == -1) {
        perror(argv[2]);
        return 1;
    }

    if (parse_octal_mode(argv[1], &new_mode)) {
/
    } else {
        new_mode = file_info.st_mode & 07777;

        if (strcmp(argv[1], "u+s") == 0) {
            new_mode |= S_ISUID;
        } else if (strcmp(argv[1], "u-s") == 0) {
            new_mode &= ~S_ISUID;
        } else if (strcmp(argv[1], "g+s") == 0) {
            new_mode |= S_ISGID;
        } else if (strcmp(argv[1], "a+rX") == 0) {
            new_mode |= S_IRUSR | S_IRGRP | S_IROTH;
            if (S_ISDIR(file_info.st_mode)
                    || (file_info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
                new_mode |= S_IXUSR | S_IXGRP | S_IXOTH;
            }
        } else {
            fprintf(stderr, "Neveljavna dovoljenja: %s\n", argv[1]);
            return 1;
        }
    }

    if (chmod(argv[2], new_mode) == -1) {
        perror(argv[2]);
        return 1;
    }

    if (stat(argv[2], &file_info) == -1) {
        perror(argv[2]);
        return 1;
    }

    format_permissions(file_info.st_mode, permissions);
    printf("Nova dovoljenja: %s\n", permissions);
    return 0;
}