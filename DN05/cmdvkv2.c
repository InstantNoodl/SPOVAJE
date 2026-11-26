#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void parse(char *niz_ukaz, char *args[])
{
    int count = 0;
    char *p = niz_ukaz;

    while (*p == ' ') {
        p++;
    }
    while (*p != '\0' && count < 32) {
        args[count++] = p;
        while (*p != '\0' && *p != ' ') {
            p++;
        }
        if (*p != '\0') {
            *p++ = '\0';
            while (*p == ' ') {
                p++;
            }
        }
    }
    args[count] = NULL;
}

int main(void)
{
    char cmd[66];
    unsigned long command_number = 0;

    while (fgets(cmd, sizeof cmd, stdin) != NULL) {
        cmd[strcspn(cmd, "\n")] = '\0';

        char *args[33];
        if (command_number % 2 == 1) {
            parse(cmd, args);
        } else {
            int count = 0;
            char *arg = strtok(cmd, " ");
            while (arg != NULL && count < 32) {
                args[count++] = arg;
                arg = strtok(NULL, " ");
            }
            args[count] = NULL;
        }
        if (args[0] == NULL) {
            continue;
        }
        command_number++;

        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            perror(args[0]);
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