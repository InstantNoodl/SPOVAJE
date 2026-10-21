#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    char buffer[4096];
    ssize_t bytes_read;
    int file_fd;
    if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'a' && argv[1][2] == '\0'){ 
    file_fd = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0644);
    }
    else{
     file_fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if (file_fd == -1) {
        perror("open");
        return 1;
   }
    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
        write(file_fd, buffer, bytes_read);
    }

    if (bytes_read == -1) {
        perror("read");
        return 1;

    }
    close(file_fd);
    return 0;
}
