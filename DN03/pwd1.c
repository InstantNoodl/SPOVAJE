#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    char path[4096] = "";

    while (1) {
        struct stat current, parent;

        if (stat(".", &current) == -1 || stat("..", &parent) == -1) {
            perror("stat");
            return 1;
        }
        if (current.st_dev == parent.st_dev && current.st_ino == parent.st_ino)
            break;

        if (chdir("..") == -1) {
            perror("chdir");
            return 1;
        }

        DIR *dir = opendir(".");

        struct dirent *entry;
        int found = 0;

        if (dir == NULL) {
            perror("opendir");
            return 1;
        }
        while ((entry = readdir(dir)) != NULL) {
            struct stat info;

            if (lstat(entry->d_name, &info) == 0 &&
                info.st_dev == current.st_dev && info.st_ino == current.st_ino) {
                size_t name_len = strlen(entry->d_name);
                size_t path_len = strlen(path);
                size_t separator = path_len != 0;

                if (name_len + separator + path_len >= sizeof(path)) {
                    fprintf(stderr, "Pot je predolga\n");
                    closedir(dir);
                    return 1;
                }
                memmove(path + name_len + separator, path, path_len + 1);
                memcpy(path, entry->d_name, name_len);
                if (separator)
                    path[name_len] = '/';
                found = 1;
                break;
            }
        }
        closedir(dir);
        if (!found) {
            fprintf(stderr, "Imena direktorija ni mogoče najti\n");
            return 1;
        }
    }

    printf("/%s\n", path);
    return 0;
}