#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    char cmd[66];

    while (fgets(cmd, sizeof cmd, stdin) != NULL) {
        cmd[strcspn(cmd, "\n")] = '\0';

        pid_t pid = fork();
        if (pid == 0) {
            execlp(cmd, cmd, (char *)NULL);
            perror(cmd);
            _exit(1);
        }
        if (pid < 0) {
            perror("fork");
            continue;
        }
        while (waitpid(pid, NULL, 0) < 0 && errno == EINTR) {}
    }

    return 0;
}