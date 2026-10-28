#include <inttypes.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

static char file_type(mode_t mode)
{
    if (S_ISREG(mode))
        return '-';
    if (S_ISDIR(mode))
        return 'd';
    if (S_ISLNK(mode))
        return 'l';
    if (S_ISFIFO(mode))
        return 'p';
    if (S_ISSOCK(mode))
        return 's';
    if (S_ISCHR(mode))
        return 'c';
    if (S_ISBLK(mode))
        return 'b';
    return '?';
}

static void file_permissions(mode_t mode, char permissions[10])
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

int main(int argc, char *argv[])
{
    struct stat file_info;
    char permissions[10];
    struct passwd *owner;

    if (argc != 2) {
        fprintf(stderr, "Uporaba: %s <datoteka>\n", argv[0]);
        return 1;
    }

    if (lstat(argv[1], &file_info) == -1) {
        perror(argv[1]);
        return 1;
    }

    file_permissions(file_info.st_mode, permissions);
    owner = getpwuid(file_info.st_uid);
    if (owner != NULL) {
        printf("%c%s %ju %s %" PRIdMAX " %s\n",
               file_type(file_info.st_mode),
               permissions,
               (uintmax_t)file_info.st_nlink,
               owner->pw_name,
               (intmax_t)file_info.st_size,
               argv[1]);
    } else {
        printf("%c%s %ju %ju %" PRIdMAX " %s\n",
               file_type(file_info.st_mode),
               permissions,
               (uintmax_t)file_info.st_nlink,
               (uintmax_t)file_info.st_uid,
               (intmax_t)file_info.st_size,
               argv[1]);
    }

    return 0;
}