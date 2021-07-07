//
// Created by gcyganek on 13.04.2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "Inavlid argument number. Arguments are producer file, output file, row number\n");
        exit(-1);
    }


    FILE *producer_file = fopen(argv[1], "r");
    if (producer_file == NULL) {
        fprintf(stderr, "Error while opening producer file: %s\n", strerror(errno));
        exit(-1);
    }

    FILE *output_file = fopen(argv[2], "r");
    if (output_file == NULL) {
        fprintf(stderr, "Error while opening output file: %s\n", strerror(errno));
        exit(-1);
    }

    int row_num = atoi(argv[3]);

    int size = 0;
    int j = 2;
    char c;
    char *buffer = calloc(2048, sizeof(char));

    while (fread(&c, sizeof(char), 1, producer_file) != 0) {

        if (size % 2046 == 0) {
            buffer = realloc(buffer, 2048 * j);
            j++;
        }

        buffer[size] = c;
        size++;
    }
    if (buffer[size - 1] == '\n') { //redundant new lines
        buffer[size - 1] = '\0';
    }

    char *line = calloc(size + 100, sizeof(char)); // 100 if lines are not the same (what is tested)
    size_t len = 0;
    for (int i = 0; i < row_num; i++) {
        getline(&line, &len, output_file);
    }


    if (line[strlen(line) - 1] == '\n') { //redundant new lines
        line[strlen(line) - 1] = '\0';
    }
    if (strcmp(line, buffer) == 0) {
        printf("Checking passed!\n");
    } else {
        printf("Checking failed!\n");
        //printf(">%s<\n",buffer);
        //printf(">%s<",line);
    }

    free(buffer);
    free(line);
    fclose(producer_file);
    fclose(output_file);
}
