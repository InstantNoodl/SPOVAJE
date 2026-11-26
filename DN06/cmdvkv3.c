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

static void split_args(char *command, char *args[], int use_parse)
{
    if (use_parse) {
        parse(command, args);
        return;
    }

    int count = 0;
    char *arg = strtok(command, " ");
    while (arg != NULL && count < 32) {
        args[count++] = arg;
        arg = strtok(NULL, " ");
    }
    args[count] = NULL;
}

static void execute_pipe(char *left_args[], char *right_args[])
{
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return;
    }

    pid_t left_pid = fork();
    if (left_pid == 0) {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            perror("dup2");
            _exit(1);
        }
        close(pipefd[1]);
        execvp(left_args[0], left_args);
        perror(left_args[0]);
        _exit(1);
    }
    if (left_pid < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    pid_t right_pid = fork();
    if (right_pid == 0) {
        close(pipefd[1]);
        if (dup2(pipefd[0], STDIN_FILENO) < 0) {
            perror("dup2");
            _exit(1);
        }
        close(pipefd[0]);
        execvp(right_args[0], right_args);
        perror(right_args[0]);
        _exit(1);
    }
    if (right_pid < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        while (waitpid(left_pid, NULL, 0) < 0 && errno == EINTR) {}
        return;
    }

    close(pipefd[0]);
    close(pipefd[1]);
    while (waitpid(left_pid, NULL, 0) < 0 && errno == EINTR) {}
    while (waitpid(right_pid, NULL, 0) < 0 && errno == EINTR) {}
}

int main(void)
{
    char cmd[66];
    unsigned long command_number = 0;

    while (fgets(cmd, sizeof cmd, stdin) != NULL) {
        cmd[strcspn(cmd, "\n")] = '\0';

        char *pipe_separator = strstr(cmd, "=>");
        if (pipe_separator != NULL) {
            if (strstr(pipe_separator + 2, "=>") != NULL) {
                fprintf(stderr, "only one pipe is supported\n");
                continue;
            }

            *pipe_separator = '\0';
            char *right_command = pipe_separator + 2;
            char *left_args[33];
            char *right_args[33];
            int use_parse = command_number % 2 == 1;
            split_args(cmd, left_args, use_parse);
            split_args(right_command, right_args, use_parse);
            if (left_args[0] == NULL || right_args[0] == NULL) {
                fprintf(stderr, "pipe requires two commands\n");
                continue;
            }

            command_number++;
            execute_pipe(left_args, right_args);
            continue;
        }

        char *args[33];
        split_args(cmd, args, command_number % 2 == 1);
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