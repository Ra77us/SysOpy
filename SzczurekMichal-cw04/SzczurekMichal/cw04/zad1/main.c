#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

#define IGNORE 0
#define HANDLER 1
#define MASK 2
#define PENDING 3

void handler(int sig_no) {
    printf("Received signal in process %d\n", getpid());
}

int main(int argc, char **argv) {
    int mode = -1;

    // Input Validation
    if (argc != 3) {
        fprintf(stderr, "Invalid number of arguments. Example arguments are: handler, fork (2)");
        exit(-1);
    }
    if (strcmp(argv[1], "ignore") == 0) {
        mode = IGNORE;
    } else if (strcmp(argv[1], "handler") == 0) {
        mode = HANDLER;
    } else if (strcmp(argv[1], "mask") == 0) {
        mode = MASK;
    } else if (strcmp(argv[1], "pending") == 0) {
        mode = PENDING;
    } else {
        fprintf(stderr, "Invalid mode\n");
        exit(-1);
    }
    if (strcmp(argv[2], "fork") != 0 && strcmp(argv[2], "exec") != 0) {
        fprintf(stderr, "Invalid second argument\n");
        exit(-1);
    }

    // IGNORE
    if (mode == IGNORE) {
        signal(SIGUSR1, SIG_IGN);
        raise(SIGUSR1);
        if (strcmp(argv[2], "fork") == 0) {
            if (fork() == 0) {
                raise(SIGUSR1);
            }
        }
        else {
            execl("./exec", "./exec", argv[1], (char *) 0);
        }
    }

    //HANDLER
    else if (mode == HANDLER) {
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_handler = handler;
        act.sa_flags = 0;
        if (sigaction(SIGUSR1, &act, NULL) == -1) {
            fprintf(stderr, "Error while setting maks\n");
            exit(-1);
        }
        raise(SIGUSR1);
        if (strcmp(argv[2], "fork") == 0) {
            if (fork() == 0) {
                raise(SIGUSR1);
            }
        }
        else {
            execl("./exec", "./exec", argv[1], (char *) 0);
        }
    }

    //MASK / PENDING
    else {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
            fprintf(stderr, "Error while setting maks\n");
            exit(-1);
        }
        raise(SIGUSR1);
        sigset_t pending_set;
        sigpending(&pending_set);
        if (sigismember(&pending_set, SIGUSR1)) {
            printf("Signal pending in parent process (%d)\n", getpid());
        }
        else {
            printf("Signal is NOT pending in parent process (%d)\n", getpid());
        }

        if (strcmp(argv[2], "fork") == 0) {
            if (fork() == 0) {
                if (mode == MASK) {
                    raise(SIGUSR1);
                }
                sigpending(&pending_set);
                if (sigismember(&pending_set, SIGUSR1)) {
                    printf("Signal pending in child process (%d)\n", getpid());
                } else {
                    printf("Signal is NOT pending in child process (%d)\n", getpid());
                }
            }
        }
        else {
            execl("./exec", "./exec", argv[1], (char *) 0);
        }
    }
    wait(NULL);
}
