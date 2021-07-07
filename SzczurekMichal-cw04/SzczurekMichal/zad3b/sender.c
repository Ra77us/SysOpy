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
#define SIG1 mode == SIGRT ? SIGRTMIN + 5 : SIGUSR1
#define SIG2 mode == SIGRT ? SIGRTMIN + 6 : SIGUSR2

int sig_count = 0;
int sig2_received = 0;
int catcher_count;
int ready = 1;

void usr1_handler(int signum, siginfo_t *info, void *ucontext) {
    sig_count++;
    if (mode == SIGQUE){
        printf("Received signal %d\n", info->si_value.sival_int);
    }
    ready = 1;
}

void usr2_handler(int signum, siginfo_t *info, void *ucontext) {
        if (mode == SIGQUE){
            catcher_count = info->si_value.sival_int;
        }
     sig2_received = 1;
}

void send_signal(pid_t catcher) {
    ready = 0;
    if (mode == KILL || mode == SIGRT) {
            kill(catcher, SIG1);
    }
    else if (mode == SIGQUE) {
            union sigval v;
            sigqueue(catcher, SIGUSR1, v);
    }
}

int main(int argc, char **argv) {

    //validate input
    if (argc != 4) {
        fprintf(stderr, "Invalid number of arguments\n");
        exit(-1);
    }

    pid_t catcher_pid = atoi(argv[1]);
    int n = atoi(argv[2]);
    
    if (strcmp(argv[3], "KILL")==0) {
        mode = KILL;
    } else if (strcmp(argv[3], "SIGQUE")==0) {
        mode = SIGQUE;
    } else if (strcmp(argv[3], "SIGRT")==0) {
        mode = SIGRT;
    } else {
        fprintf(stderr, "Invalid mode\n");
        exit(-1);
    }
    if (catcher_pid <= 0 || n <= 0) {
        fprintf(stderr, "Invalid arguments\n");
        exit(-1);
    }

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


    //send signals
    int sent = 0;
    while(sent < n){
        if(ready == 1){
            sent ++;
            send_signal(catcher_pid);
        }
    }
   
    if(mode != SIGQUE){
        kill(catcher_pid, SIG2);
    }
    else{
        union sigval v; // value is not used
        sigqueue(catcher_pid, SIGUSR2, v);
    }

    // wait for answer
    while (sig2_received == 0) {
        sigsuspend(&mask);
    }

    printf("Sender received %d signals\n", sig_count);
    if(mode == SIGQUE){
        printf("(Catcher has sent %d signals back)\n", catcher_count);
    }

}

