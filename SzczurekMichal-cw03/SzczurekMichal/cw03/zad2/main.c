#define  _GNU_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "lib1.h"
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#ifdef DLL
#include <dlfcn.h>
#endif

typedef struct time_wrapper {
    clock_t real;
    clock_t user;
    clock_t system;
} wrapper;

int is_positive_num(char *arg) {
    for (int i = 0; i < strlen(arg); i++) {
        if (!isdigit(arg[i])) {
            return 0;
        }
    }
    return 1;
}

int split_file_arg(char *arg, char **f1, char **f2) {
    int split_point = -1;
    for (int i = 0; i < strlen(arg); i++) {
        if (arg[i] == ':') {
            // there are at least two ':', such arg is invalid
            if (split_point != -1) {
                return 0;
            }
            split_point = i;
        }
    }
    if (split_point == -1 || split_point == 0 || split_point == strlen(arg) - 1)
        return 0;

    *f1 = (char *) calloc((split_point + 1), sizeof(char));
    strncpy(*f1, arg, split_point);

    *f2 = (char *) calloc(strlen(arg) - split_point, sizeof(char));
    strncpy(*f2, arg + split_point + 1, strlen(arg) - split_point - 1);

    return 1;
}

void set_times(wrapper *time_wrapper) {
    struct tms time_res;
    time_wrapper->real = times(&time_res);
    time_wrapper->user = time_res.tms_utime;
    time_wrapper->system = time_res.tms_stime;
}

void write_res(FILE *res, wrapper start, wrapper end, int argc, char** argv) {

    printf("Real time: %lf s.\n", (double) (end.real - start.real) / sysconf(_SC_CLK_TCK));
    printf("User time: %lf s.\n",  (double) (end.user - start.user) / sysconf(_SC_CLK_TCK));
    printf("System time: %lf s.\n",  (double) (end.system - start.system) / sysconf(_SC_CLK_TCK));

    fprintf(res, "\n File sequence: ");
    for (int i = 2; i < argc; i++) {
        fprintf(res, "%s ", argv[i]);
    }
    fprintf(res, "\n");
    fprintf(res, "Real time: %lf s.\n",  (double) (end.real - start.real) / sysconf(_SC_CLK_TCK));
    fprintf(res, "User time: %lf s.\n",  (double) (end.user - start.user) / sysconf(_SC_CLK_TCK));
    fprintf(res, "System time: %lf s.\n", (double) (end.system - start.system) / sysconf(_SC_CLK_TCK));
}

void merge_pair(char *pair_str) {
#ifdef DLL
    void *handle = dlopen("./liblib1.so", RTLD_LAZY);
    if(!handle){
        printf("Cannot open library");
        exit(1);
    }

    file_pair* (*new_file_pair)(char*, char* ) = dlsym(handle, "new_file_pair");
    FILE* (*merge)(file_pair*) = dlsym(handle, "merge");
#endif
    char *f1;
    char *f2;
    if (split_file_arg(pair_str, &f1, &f2) != 1) {
        fprintf(stderr, "Ignoring invalid argument: %s\n", pair_str);
        exit(-1);
    }
    file_pair *pair = new_file_pair(f1, f2);
    FILE *tmp = merge(pair);
    fclose(tmp);
    free(f1);
    free(f2);
    free(pair);
#ifdef DLL
    dlclose(handle);
#endif
}

int main(int argc, char **argv) {

    if (argc < 3) {
        fprintf(stderr,
                "Invalid number of arguments. Arguments are: file to write result to and files to merge in format a.txt:b.txt\n");
        exit(-1);
    }

    wrapper start_times;
    wrapper end_times;
    set_times(&start_times);

    if (strcmp(argv[2],"-r")==0){   //repeat mode, allows for multiple merging of same pair to perform tests more effectively

        if (argc != 5) {
            fprintf(stderr,
                    "Invalid number of arguments. Arguments in repeat mode are: res file, -r, number of repetitions, file pair\n");
            exit(-1);
        }
        int rep = atoi(argv[3]);
        if(rep < 1){
            fprintf(stderr, "Repetition number must be positive\n");
            exit(-1);
        }
        for(int i=0; i<rep; i++){
            if (fork() == 0) {
                merge_pair(argv[4]);
                exit(0);
            }
        }

    }

    else{ // normal mode
        for (int i = 2; i < argc; i++) {
            if (fork() == 0) {
                merge_pair(argv[i]);
                exit(0);
            }
        }
    }

    while (wait(NULL) != -1) {}

    set_times(&end_times);

    FILE *res_f = fopen(argv[1], "a");
    if (res_f == NULL) {
        printf("Cannot open result file.\n");
        exit(1);
    }
    write_res(res_f, start_times, end_times, argc, argv);
    fclose(res_f);
}


