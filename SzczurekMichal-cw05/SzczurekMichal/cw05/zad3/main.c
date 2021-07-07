#define  _GNU_SOURCE


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 4 && argc != 5) {
        fprintf(stderr,
                "Invalid number of arguments- required are: producers num, consumer num, N, optional flag for big files\n");
        exit(-1);
    }


    mkfifo("FIFO_ID", 0666);


    int producer_num = atoi(argv[1]);
    int consumer_num = atoi(argv[2]);

    if (producer_num < 1 || producer_num > 5) {
        fprintf(stderr, "Invalid number of producers, must be 1-5\n");
        exit(-1);
    }
    if (consumer_num < 1 || consumer_num > 5) {
        fprintf(stderr, "Invalid number of consumers, must be 1-5\n");
        exit(-1);
    }

    char *file_name = calloc(30, sizeof(char));
    for (int i = 1; i <= producer_num; i++) {
        char i_str[2];
        sprintf(i_str, "%d", i);
        if (fork() == 0) {
            if (argc != 5) {
                sprintf(file_name, "producer_file_%d.txt", i);
            } else {
                sprintf(file_name, "producer_file_big_%d.txt", i);
            }

            if (execlp("./producer", "producer", "FIFO_ID", i_str, file_name, argv[3], NULL) == -1) {
                fprintf(stderr, "Error while using exec: %s\n", strerror(errno));
            }
        }

    }


    for (int i = 1; i <= consumer_num; i++) {
        if (fork() == 0) {

            if (execlp("./consumer", "consumer", "FIFO_ID", "output_file.txt", argv[3], NULL) == -1) {
                fprintf(stderr, "Error while using exec: %s\n", strerror(errno));
            }
        }
    }

    while (wait(NULL) != -1) {};
    free(file_name);


}

