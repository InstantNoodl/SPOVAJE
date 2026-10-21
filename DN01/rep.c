#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
   ssize_t bytes_read;
   int file_fd;
   int n=5;
   if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'n' && argv[1][2] == '\0'){
    
    n=atoi(argv[2]);
    file_fd = open(argv[3], O_RDONLY, 0644);

   }else{
   file_fd = open(argv[1], O_RDONLY, 0644);
   }
    if (file_fd == -1) {
         perror("open");
         return 1;
    }
    char ch;
    int count = 0;
    int i=2;
    
    while ((bytes_read = read(file_fd, &ch, 1)) == 1) {
        lseek(file_fd, -sizeof(char)*i, SEEK_END);
        i++;
       
        if (ch == '\n') {
         count++;
        }
        if (count == n) {
            break;
        }
    }
    lseek(file_fd, -sizeof(char)*(i-3), SEEK_END);
    while ((bytes_read = read(file_fd, &ch, 1)) == 1) {
         if (write(STDOUT_FILENO, &ch, 1) == -1) {
            perror("write");
            close(file_fd);
            return 1;
        }
    }
    if (bytes_read == -1) {
        perror("read");
        close(file_fd);
        return 1;
    }
  

  

    return 0;
}
