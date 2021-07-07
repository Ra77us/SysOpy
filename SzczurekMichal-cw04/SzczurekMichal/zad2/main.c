#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>


void handler_info(int signum, siginfo_t *info, void *ucontext) {

    printf("Signal received!\n");
    printf("Signal number %d\n", info->si_signo);
    printf("Sending process ID %d\n", info->si_pid);

    if (signum == SIGTERM) {
        printf("Value sent along signal: %d\n", info->si_value.sival_int);
    }

    if (signum == SIGCHLD) {
        printf("Child exit value: %d\n", info->si_status);
    } else {
        printf("Signal sent by ");
        if (info->si_code == SI_USER) {
            printf("user (using kill)\n");
        } else if (info->si_code == SI_KERNEL) {
            printf("kernel\n");
        } else if (info->si_code == SI_QUEUE) {
            printf("signal queue\n");
        }
    }
}

int signal_no = 0;
void handler_nodefer(int sig_no) {
    int id = signal_no;
    printf("Starting handler for signal number %d \n", id);
    if (signal_no != 2) {
        signal_no++;
        raise(SIGUSR1);
    }
    printf("Ending handler for signal number %d \n", id);
}

void handler_reset(int sig_no) {
    printf("Signal received (SIGUSR2)!\n");
}

int main(int argc, char **argv) {

    printf("------- Now showing effect of SA_SIGINFO -------\n\n");
    struct sigaction act_info;
    act_info.sa_flags = SA_SIGINFO;
    act_info.sa_sigaction = handler_info;
    sigemptyset(&act_info.sa_mask);
    if (sigaction(SIGALRM, &act_info, NULL) == -1) {
        fprintf(stderr, "Error while setting sigaction");
        exit(-1);
    }
    if (sigaction(SIGCHLD, &act_info, NULL) == -1) {
        fprintf(stderr, "Error while setting sigaction");
        exit(-1);
    }
    if (sigaction(SIGTERM, &act_info, NULL) == -1) {
        fprintf(stderr, "Error while setting sigaction");
        exit(-1);
    }

    printf(">>>> Signal 1 - SIGALRM\n");
    kill(getpid(), SIGALRM);

    printf("\n\n>>>> Signal 2 - SIGCHLD with child status 10\n");
    if (fork() == 0) {
        exit(10);
    }
    wait(NULL);

    printf("\n>>>> Signal 3 - SIGTERM with value 10 in sigque\n");
    union sigval val;
    val.sival_int = 10;
    sigqueue(getpid(), SIGTERM, val);


    printf("\n\n------- Now showing effect of SA_NODEFER -------\n");
    printf("------- (signals will be send in handler) -------\n\n");

    printf("\n>> No flags (for comparison):\n\n");
    struct sigaction default_defer;
    default_defer.sa_flags = 0;
    default_defer.sa_handler = handler_nodefer;
    sigemptyset(&default_defer.sa_mask);
    if (sigaction(SIGUSR1, &default_defer, NULL) == -1) {
        fprintf(stderr, "Error while setting sigaction");
        exit(-1);
    }

    raise(SIGUSR1);

    printf("\n>> With SA_NODEFER:\n\n");
    struct sigaction act_nodefer;
    act_nodefer.sa_flags = SA_NODEFER;
    act_nodefer.sa_handler = handler_nodefer;
    sigemptyset(&act_nodefer.sa_mask);
    if (sigaction(SIGUSR1, &act_nodefer, NULL) == -1) {
        fprintf(stderr, "Error while setting sigaction");
        exit(-1);
    }

    signal_no = 0;
    raise(SIGUSR1);


    printf("\n\n------- Now showing effect of SA_RESTHAND -------");
    printf("\n------- Aftrer second signal process will be terminated -------\n\n");

    printf("\n>> No flags (for comparison):\n\n");
    struct sigaction default_reset;
    default_reset.sa_flags = 0;
    default_reset.sa_handler = handler_reset;
    sigemptyset(&default_reset.sa_mask);
    if (sigaction(SIGUSR2, &default_reset, NULL) == -1) {
        fprintf(stderr, "Error while setting sigaction");
        exit(-1);
    }

    raise(SIGUSR2);
    raise(SIGUSR2);

    printf("\n>> With SA_RESTHAND:\n\n");
    struct sigaction act_reset;
    act_reset.sa_flags = SA_RESETHAND;
    act_reset.sa_handler = handler_reset;
    sigemptyset(&act_reset.sa_mask);
    if (sigaction(SIGUSR2, &act_reset, NULL) == -1) {
        fprintf(stderr, "Error while setting sigaction");
        exit(-1);
    }

    raise(SIGUSR2);
    printf("------- Program will end now due to default handling-------\n");
    raise(SIGUSR2);

}
