#define  _GNU_SOURCE


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include <errno.h>


void write_to_file(FILE *file, char *text, int target_row) {

    rewind(file);
    int row_count = 0;
    int size = 0;
    int j = 2;
    char c;
    char *buffer = calloc(2048, sizeof(char));
    int written = 0;

    while (fread(&c, sizeof(char), 1, file) != 0) {

        if (size % 2046 == 0) {
            buffer = realloc(buffer, 2048 * j);
            j++;
        }

        buffer[size] = c;
        size++;
    }

    rewind(file);

    for (int i = 0; i < size; i++) {
        c = buffer[i];
        if (c == '\n') {
            row_count++;
        }
        if (target_row + 1 == row_count && written != 1) {
            fwrite(text, sizeof(char), strlen(text), file);
            written = 1;
        }
        fwrite(&c, sizeof(char), 1, file);

    }

    c = '\n';
    if (written != 1) {
        while (row_count < target_row) {
            fwrite(&c, sizeof(char), 1, file);
            row_count++;
        }
        fwrite(text, sizeof(char), strlen(text), file);

    }


    free(buffer);
    rewind(file);
}


int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, "Invalid number of arguments\n");
        exit(-1);
    }

    FILE *fifo = fopen(argv[1], "r");
    if (fifo == NULL) {
        fprintf(stderr, "Error while opening file %s\n", strerror(errno));
        exit(-1);
    }

    FILE *file = fopen(argv[2], "w+");
    if (file == NULL) {
        fprintf(stderr, "Error while opening file %s\n", strerror(errno));
        exit(-1);
    }

    char *line;
    size_t len = 0;
    while (getline(&line, &len, fifo) != -1) {
        if (strlen(line) == 1) //empty new line at the end
            break;

        char *token = strtok(line, "#");
        int row_num = atoi(token) - 1; //since we numerate rows from 1

        token = strtok(NULL, "#");

        char *text = calloc(strlen(token) + 1, sizeof(char));
        strncpy(text, token, strlen(token) - 1);

        flock(fileno(file), LOCK_EX);
        write_to_file(file, text, row_num);
        flock(fileno(file), LOCK_UN);

        free(text);
    }
    free(line);
    fclose(fifo);
    fclose(file);
}
