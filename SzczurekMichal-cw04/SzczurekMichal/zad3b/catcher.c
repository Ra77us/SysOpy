#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// 0- KILL, 1- SIGQUE, 2- SIGRT
#define KILL 0
#define SIGQUE 1
#define SIGRT 2
int mode = -1;
#define SIG1 mode == SIGRT ? SIGRTMIN + 5: SIGUSR1
#define SIG2 mode == SIGRT ? SIGRTMIN + 6 : SIGUSR2

int sig_count = 0;
int sig2_received = 0;
pid_t sender;


void confirm(pid_t sender){
    if (mode == KILL || mode == SIGRT) {
        kill(sender, SIG1);

    } else if (mode == SIGQUE) {
        union sigval v;
        v.sival_int = sig_count-1;
        sigqueue(sender, SIGUSR1, v);
    }
}

void usr1_handler(int signum, siginfo_t *info, void *ucontext) {
    sig_count += 1;
    sender = info->si_pid;
    confirm(info->si_pid);
}


void usr2_handler(int signum, siginfo_t *info, void *ucontext) {
    sig2_received = 1;
    sender = info->si_pid;
}


int main(int argc, char** argv) {

    // validate input
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments (mode needs to be specified).\n");
        exit(-1);
    }
    if (strcmp(argv[1], "KILL")==0) {
        mode = KILL;
    } else if (strcmp(argv[1], "SIGQUE")==0) {
        mode = SIGQUE;
    } else if (strcmp(argv[1], "SIGRT")==0) {
        mode = SIGRT;
    } else {
        fprintf(stderr, "Invalid mode\n");
        exit(-1);
    }

    printf("My PID is: %d\n", getpid());


    //set mask and handlers
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIG1);
    sigdelset(&mask, SIG2);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    struct sigaction act_usr1;
    act_usr1.sa_flags = SA_SIGINFO;
    act_usr1.sa_sigaction = usr1_handler;
    if (sigaction(SIG1, &act_usr1, NULL) == -1) {
        fprintf(stderr, "Error is sigaction for SIG1\n");
        exit(-1);
    }

    struct sigaction act_usr2;
    act_usr2.sa_flags = SA_SIGINFO;
    act_usr2.sa_sigaction = usr2_handler;
    if (sigaction(SIG2, &act_usr2, NULL) == -1) {
        fprintf(stderr, "Error is sigaction for SIG2\n");
        exit(-1);
    }

    // wait for signals
    while (sig2_received == 0) {
        sigsuspend(&mask);
    }

    // count signals + send back
    int received = sig_count; // so as not to increase sig_count during sending signals to sender
    // The counting has ended.
    printf("Catcher received %d signals\n", received);
    if (mode == KILL || mode == SIGRT) {
        kill(sender, SIG2);

    } else if (mode == SIGQUE) {
        union sigval counter;
        counter.sival_int = received;
        sigqueue(sender, SIGUSR2, counter);

    }
}

