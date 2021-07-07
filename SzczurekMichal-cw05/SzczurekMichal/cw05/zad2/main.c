#define  _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char** argv) {

    char* command;
    FILE* pipe;

    if(argc == 2){
    	command = calloc(32, sizeof(char));
        if (strcmp(argv[1], "data")==0){
            sprintf(command, "mail -H | sort -k5,5M -k6,7");
            pipe =  popen(command, "w");
            if (pipe == NULL) {
                fprintf(stderr, "Error occurred while piping: %s", strerror(errno));
                free(command);
                return 1;
            }
        }
        else if (strcmp(argv[1], "nadawca")==0){
            sprintf(command, "mail -H | sort -k3,3");
            pipe =  popen(command, "w");
            if (pipe == NULL) {
                fprintf(stderr, "Error occurred while piping: %s", strerror(errno));
                free(command);
                return 1;
            }
        }
        else{
            fprintf(stderr, "Invalid argument\n");
            free(command);
            exit(-1);
        }
    }
    else if (argc == 4){
	int len = strlen(argv[1]) +  strlen(argv[2]) + strlen(argv[3]);
	command = calloc(32 + len, sizeof(char));
	sprintf(command, "echo %s | mail %s -s %s", argv[3], argv[1], argv[2]);
	pipe =  popen(command, "w");
            if (pipe == NULL) {
                fprintf(stderr, "Error occurred while piping: %s", strerror(errno));
                free(command);
                return 1;
       }
	printf("Mail has been sent!\n");
    }
    else{
        fprintf(stderr, "Invalid number of arguments\n");
        exit(-1);
    }
     free(command);
     pclose(pipe);
}

