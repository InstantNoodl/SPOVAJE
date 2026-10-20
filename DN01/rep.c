#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(STDERR_FILENO, "Usage: rep FILE\n", 16);
        return 1;
    }

    int file_fd = open(argv[1], O_RDONLY);
    if (file_fd == -1) {
        write(STDERR_FILENO, "rep: cannot open file\n", 22);
        return 1;
    }

    off_t end = lseek(file_fd, 0, SEEK_END);
    if (end == (off_t)-1) {
        write(STDERR_FILENO, "rep: lseek failed\n", 18);
        close(file_fd);
        return 1;
    }

    if (end > 0) {
        char last_character;
        if (lseek(file_fd, -1, SEEK_END) == (off_t)-1 ||
            read(file_fd, &last_character, 1) != 1) {
            write(STDERR_FILENO, "rep: read failed\n", 17);
            close(file_fd);
            return 1;
        }

        if (write(STDOUT_FILENO, &last_character, 1) != 1) {
            write(STDERR_FILENO, "rep: write failed\n", 18);
            close(file_fd);
            return 1;
        }
    }

    if (close(file_fd) == -1) {
        write(STDERR_FILENO, "rep: close failed\n", 18);
        return 1;
    }

    return 0;
}
