#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int main(int argc,char* argv[]){
    if (argc == 2){
        int n = atoi(argv[1]);
        pid_t child;
        for (int i=0; i<n; i++){
            child = fork();
            if(child == 0){
                printf("Process %d created by %d\n",getpid(), getppid());
                return 0;
            }
            else{
                wait(NULL);
            }
        }
    }
    else{
        fprintf(stderr, "Invalid argument number\n");
        exit(-1);
    }
}