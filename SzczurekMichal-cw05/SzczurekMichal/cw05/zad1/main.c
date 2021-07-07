#define  _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define MAX_COMPONENTS_IN_LINE 100
#define MAX_ARGS 10
#define MAX_ARG_LEN 100
#define MAX_COMMAND_LEN 100


void execute(FILE *file, char **components) {
    char *line;
    size_t line_size = 0;
    getline(&line, &line_size, file);
    int j = 0;
    char num[10];
    int curr_fd[2];
    int prev_fd[2];
    int first_in_pipe = 1;
    int last_in_pipe = 0;

    while (getline(&line, &line_size, file) != -1) {
        printf("\n=========================================\n");
        int components_in_line = 0;
        first_in_pipe = 1;
        last_in_pipe = 0;
        int line_commands[MAX_COMPONENTS_IN_LINE];

        //------------- Parse command to ints ex składnik1 | składnik7 -> [0,6]
        for (int i = 0; line[i] != '\0'; i++) {

            if (i > 0 && line[i] == 'k' && line[i - 1] == 'i') {// ...ik<NUM>
                memset(num, 0, 10);
                j = 0;
                i++;
                while (line[i] != '\0' && line[i] != ' ' && line[i] != '\n') {
                    num[j] = line[i];
                    j++;
                    i++;
                }
                line_commands[components_in_line] = atoi(num) - 1;
                components_in_line++;
            }
        }
        //------------- Execute components
        for (int i = 0; i < components_in_line; i++) {

            char *curr_component = components[line_commands[i]];
            char *component_cpy = calloc(strlen(curr_component) + 1, sizeof(char));
            strcpy(component_cpy, curr_component);

            char *token = strtok_r(component_cpy, "|", &component_cpy);
            printf("Now executing command: %s\n", components[line_commands[i]]);

            // ----- Split into single commands
            while (token != NULL) {
                char *next = strtok_r(component_cpy, "|", &component_cpy);
                if (next == NULL && i == components_in_line - 1) {
                    last_in_pipe = 1;
                    printf("--------------\n");

                }
                char *command = calloc(MAX_COMMAND_LEN, sizeof(char));
                char *args[MAX_ARGS + 2];
                int argc = 0;
                char *el = strtok(token, " ");
                strcpy(command, el);
                while (el != NULL && argc < MAX_ARGS + 1) {
                    args[argc] = calloc(MAX_ARG_LEN, sizeof(char));
                    strcpy(args[argc], el);
                    argc++;
                    el = strtok(NULL, " ");
                }
                args[argc] = NULL;

                // --- Execute single command
                pipe(curr_fd);
                pid_t child = fork();

                if (child == 0) {
                    if (first_in_pipe != 1) {
                        printf("d");
                        dup2(prev_fd[0], STDIN_FILENO);
                        close(prev_fd[0]);
                    }
                    if (last_in_pipe != 1) {
                        dup2(curr_fd[1], STDOUT_FILENO);
                    }

                    if (execvp(command, args) == -1) {
                        fprintf(stderr, "Error in exec: %s\n", strerror(errno));
                        exit(-1);
                    }
                }
                close(curr_fd[1]);

                while (wait(NULL) != -1) {};
                if (first_in_pipe == 1) {
                    first_in_pipe = 0;
                } else {
                    close(prev_fd[0]);
                }
                prev_fd[0] = curr_fd[0];
                prev_fd[1] = curr_fd[1];

                // ---cleanup
                free(command);
                for (int j = 0; j < argc; j++) {
                    free(args[j]);
                }

                if (next != NULL) {
                    strcpy(token, next);
                } else {
                    free(token);
                    token = NULL;
                }
            }


        }
        while (wait(NULL) != -1) {};
    }

    free(line);
}

int main(int argc, char **argv) {

    // ----------------- Error handling
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments\n");
        exit(-1);
    }
    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", strerror(errno));
        exit(-1);
    }

    // ------------------ Parsing components
    int component_count = 0;
    char *line;
    size_t line_size = 0;
    int flag = 0;
    while (getline(&line, &line_size, file) != -1) {
        if (line[0] != 's') {
            flag = 1;
        } else if (flag == 0) {
            component_count++;
        }
    }
    rewind(file);
    char **components = calloc(component_count, sizeof(char *));
    for (int i = 0; i < component_count; i++) {
        getline(&line, &line_size, file);
        char *token = strtok(line, "=");
        token = strtok(NULL, "=");
        components[i] = calloc(strlen(token) + 1, sizeof(char));
        strncpy(components[i], token + 1, strlen(token) - 2);
    }
    //------------ Executing commands
    execute(file, components);

    //------------ Free
    for (int i = 0; i < component_count; i++) {
        free(components[i]);
    }
    free(line);
    free(components);
    fclose(file);
}

