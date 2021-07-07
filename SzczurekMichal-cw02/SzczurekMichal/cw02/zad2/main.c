#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
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

char *my_getline(MYFILE file, char character, int *empty, int *has_char) {
    *has_char = 0;
    char *line = (char *) calloc(250, sizeof(char));
    int i = 0;
    while (READ) {
        if (line[i] == '\n') {
            line[i + 1] = '\0';
            return line;
        }

        if (line[i] == character)
            *has_char = 1;

        i++;
        if (i % 249 == 0) {
            line = realloc(line, (i + 250) * sizeof(char));
        }
    }
    *empty = 1;
    line[i + 1] = '\0';
    return line;
}

void read_lines(char character, char *file) {

#ifndef SYS
    MYFILE f = fopen(file, "r");
    if (f == NULL) {
       fprintf(stderr,"Cannot open file(s) %s\n", strerror(errno));
        exit(-1);
    }
#endif

#ifdef SYS
    MYFILE f = open(file, O_RDONLY);
     if (f < 0 ){
       fprintf(stderr,"Cannot open file(s) %s\n", strerror(errno));
        exit(-1);
    }
#endif

    int empty = 0;
    int has_char = 0;
    char *line;
    while (!empty) {
        line = my_getline(f, character, &empty, &has_char);
        if (has_char) {
            printf("%s", line);
        }
        free(line);
    }
    CLOSE(f);
}

int main(int argc, char **argv) {

    char file[250];
    char character;

    if (argc == 1) {
        printf("Provide character and file name\n");
        scanf("%c", &character);
        scanf("%s", file);
    } else {
        if (strlen(argv[1]) != 1) {
            fprintf(stderr,"First argument has to be single character\n");
            exit(-1);
        }
        character = argv[1][0];

        if (argc == 2) {
            printf("Provide file name\n");
            scanf("%s", file);
        } else if (argc == 3) {
            strcpy(file, argv[2]);
        } else {
            fprintf(stderr,"Too many arguments\n");
            exit(-1);
        }
    }

#ifdef TIME
    double user_time;
    double system_time;
    double real_time;
    time_wrapper wrapper;
    set_times(&wrapper);
#endif

    read_lines(character, file);

#ifdef TIME
    get_times(&wrapper, &real_time, &user_time, &system_time);
    printf("Real time: %f\n",real_time);
    printf("User time: %f\n",user_time);
    printf("System time: %f\n\n",system_time);
   

    FILE* res = fopen("pomiar_zad_2.txt", "a");
    if(res == NULL){
          fprintf(stderr,"Cannot open result file%s\n", strerror(errno));
    }
    else{
        fprintf(res, "Times for %c character and %s file\n",character, file);
        fprintf(res, "Real time: %f\n",real_time);
        fprintf(res, "User time: %f\n",user_time);
        fprintf(res, "System time: %f\n\n",system_time);
    }
    fclose(res);
#endif
}

