#include <signal.h>
#include <unistd.h>

void handle_sigint(int sig)
{
    (void)sig;
    write(STDOUT_FILENO, "SIGINT caught\n", 13);
}

void wait_seconds(unsigned int seconds)
{
    while ((seconds = sleep(seconds)) != 0) {}
}

int main(void)
{
    signal(SIGINT, handle_sigint);
    wait_seconds(5);
    signal(SIGINT, SIG_IGN);
    wait_seconds(5);
    signal(SIGINT, SIG_DFL);
    wait_seconds(5);
    return 0;
}