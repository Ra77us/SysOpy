
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/file.h>

int main(int argc, char *argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Invalid number of arguments\n");
        exit(-1);
    }

    FILE *pipe = fopen(argv[1], "w");
    if (pipe == NULL) {
        fprintf(stderr, "Error while opening pipe: %s\n", strerror(errno));
        exit(-1);
    }

    int row_num = atoi(argv[2]);

    FILE *file = fopen(argv[3], "r");

    if (file == NULL) {
        fprintf(stderr, "Error while opening file: %s\n", strerror(errno));
        exit(-1);
    }

    int N = atoi(argv[4]);

    char *buffer = calloc(N + 1, sizeof(char));

    srand(time(NULL));

    int loaded = fread(buffer, sizeof(char), N, file);
    while (loaded > 0) {

        sleep(rand() % 2 + 1);
        buffer[loaded] = '\0';
        char *input = calloc(loaded + 4, sizeof(char));
        if (buffer[0] == '\n') {
            break;
        }

        if (buffer[loaded - 1] == '\n') {
            buffer[loaded - 1] = '\0';
        }


        sprintf(input, "%d#%s\n", row_num, buffer);
        flock(fileno(pipe), LOCK_EX);

        fwrite(input, sizeof(char), strlen(input), pipe);
        flock(fileno(pipe), LOCK_UN);
        free(input);
        loaded = fread(buffer, sizeof(char), N, file);
    }

    free(buffer);
    fclose(file);
    fclose(pipe);
}
