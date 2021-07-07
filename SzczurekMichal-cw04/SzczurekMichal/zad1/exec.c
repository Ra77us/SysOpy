#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define IGNORE 0
#define HANDLER 1
#define MASK 2
#define PENDING 3

int main(int argc, char** argv){

    int mode;
    if (strcmp(argv[1], "ignore") == 0){
        mode = IGNORE;
    }
    else if(strcmp(argv[1], "handler") == 0){
        mode = HANDLER;
    }
    else if(strcmp(argv[1], "mask") == 0){
        mode = MASK;
    }
    else if(strcmp(argv[1], "pending") == 0){
        mode = PENDING;
    }

    sigset_t pending_set;
    if (mode != PENDING){
        raise(SIGUSR1);
    }
    if (mode == MASK || mode == PENDING){
        sigpending(&pending_set);
        if (sigismember(&pending_set, SIGUSR1)){
            printf("Signal pending in exec process (%d)\n",getpid());
        }
        else{
            printf("Signal is NOT pending in exec process (%d)\n",getpid());
        }
    }
}