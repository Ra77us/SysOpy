#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handler(int signo, siginfo_t *siginfo, void *context){
    printf("Passed value: %d, Signal: %d\n", siginfo->si_value.sival_int, signo);
}


int main(int argc, char* argv[]) {

    if(argc != 3){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    
    sigaction(SIGUSR1,&action,NULL);
    sigaction(SIGUSR2,&action,NULL);
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigdelset(&set, SIGUSR2);
    if(sigprocmask(SIG_BLOCK,&set,NULL)<0){
         printf("error while setting mask\n");
         exit(-1);
    }
    //..........
    //zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1 i SIGUSR2
    ///zdefiniuj obsluge SIGUSR1 i SIGUSR2 w taki sposob zeby proces potomny wydrukowal
    //na konsole przekazana przez rodzica wraz z sygnalami SIGUSR1 i SIGUSR2 wartosci

    int child = fork();
    if(child == 0) {
        sleep(1); // żeby dziecko nie skońcyzło się za wcześnie
    }
    else {
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
        union sigval sv;
   	sv.sival_int = atoi(argv[1]);
        sigqueue(child, atoi(argv[2]), sv);
    }

    return 0;
}
