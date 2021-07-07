#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <errno.h>

#ifdef SYS
#define MYFILE int
#define READ read(file, &line[i], sizeof(char))
#define CLOSE close
#endif

#ifndef SYS
#define MYFILE FILE*
#define READ fread(&line[i], sizeof(char), 1, file)
#define CLOSE fclose
#endif

typedef struct time_wrapper {
    clock_t start_real;
    clock_t start_user;
    clock_t start_system;
} time_wrapper;

void set_times(time_wrapper *wrapper) {
    struct tms time_res;
    wrapper->start_real = times(&time_res);
    wrapper->start_user = time_res.tms_utime;
    wrapper->start_system = time_res.tms_stime;
}

void get_times(time_wrapper *wrapper, double *real_time, double *user_time, double *system_time) {
    struct tms time_res;
    clock_t curr;
    curr = times(&time_res);

    *real_time = (double) (curr - wrapper->start_real) / sysconf(_SC_CLK_TCK);
    *user_time = (double) (time_res.tms_utime - wrapper->start_user) / sysconf(_SC_CLK_TCK);
    *system_time = (double) (time_res.tms_stime - wrapper->start_system) / sysconf(_SC_CLK_TCK);
}


char *my_getline(MYFILE file, int *empty) {
    char *line = (char *) calloc(250, sizeof(char));
    int i = 0;
    while (READ) {
        if (line[i] == '\n') {
            line[i + 1] = '\0';
            return line;
        }

        i++;
        if (i % 248 == 0) {
            line = (char *) realloc(line, (i + 250) * sizeof(char));
        }
    }
    *empty = 1;
    line[i + 1] = '\0';
    return line;
}

void merge(char *file1, char *file2) {

#ifndef SYS
    MYFILE f1 = fopen(file1, "r");
    MYFILE f2 = fopen(file2, "r");
    if (f1 == NULL || f2 == NULL) {
        fprintf(stderr,"Cannot open file(s) %s\n", strerror(errno));
        exit(-1);
    }
#endif

#ifdef SYS
    MYFILE f1 = open(file1, O_RDONLY);
    MYFILE f2 = open(file2, O_RDONLY);
     if (f1 < 0 || f2 < 0 ){
        fprintf(stderr,"Cannot open file(s)%s\n", strerror(errno));
        exit(-1);
    }
#endif

    int first_empty = 0;
    int second_empty = 0;
    char *line;

    while (!(first_empty && second_empty)) {
        if (!first_empty) {
            line = my_getline(f1, &first_empty);
            printf("%s", line);
            free(line);
        }
        if (!second_empty) {
            line = my_getline(f2, &second_empty);
            printf("%s", line);
            free(line);
        }
    }

    CLOSE(f1);
    CLOSE(f2);
}

int main(int argc, char **argv) {

    char file1[250];
    char file2[250];

    if (argc == 1) {
        printf("Provide files to merge\n");
        scanf("%s", file1);
        scanf("%s", file2);
    } else if (argc == 2) {
        strcpy(file1, argv[1]);
        printf("Provide one more file\n");
        scanf("%s", file2);
    } else if (argc == 3) {
        strcpy(file1, argv[1]);
        strcpy(file2, argv[2]);
    } else {
        fprintf(stderr,"Too many arguments\n");
        exit(-1);
    }

#ifdef TIME
    double user_time;
    double system_time;
    double real_time;
    time_wrapper wrapper;
    set_times(&wrapper);
#endif

    merge(file1, file2);

#ifdef TIME
    get_times(&wrapper, &real_time, &user_time, &system_time);
    printf("Real time: %f\n",real_time);
    printf("User time: %f\n",user_time);
    printf("System time: %f\n\n",system_time);
   

    FILE* res = fopen("pomiar_zad_1.txt", "a");
    if(res == NULL){
        fprintf(stderr,"Cannot open result file%s\n", strerror(errno));
    }
    else{
        fprintf(res, "Times for %s and %s\n",file1, file2);
        fprintf(res, "Real time: %f\n",real_time);
        fprintf(res, "User time: %f\n",user_time);
        fprintf(res, "System time: %f\n\n",system_time);
    }
    fclose(res);
#endif
}

