#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#ifdef SYS
#define MYFILE int
#define READ read(file, &line[i], sizeof(char))
#define WRITE write(file, line, strlen(line))
#define CLOSE close
#endif

#ifndef SYS
#define MYFILE FILE*
#define READ fread(&line[i], sizeof(char), 1, file)
#define WRITE fwrite(line, sizeof(char), strlen(line), file)
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

int is_positive_num(char *arg) {
    for (int i = 0; i < strlen(arg); i++) {
        if (!isdigit(arg[i])) {
            return 0;
        }
    }
    return 1;
}

char *my_getline(MYFILE file, int *empty) {
    char *line = (char *) calloc(13, sizeof(char));
    int i = 0;
    while (READ) {
        if (line[i] == '\n') {
            line[i] = '\0';
            return line;
        }
        i++;
        if (i % 11 == 0) {
            fprintf(stderr,"Number too big (for int)\n");
            exit(-1);
        }

    }
    *empty = 1;
    line[i + 1] = '\0';
    return line;
}

void open_files(MYFILE*data, MYFILE*res1, MYFILE*res2, MYFILE*res3, char *data_name) {


    char *res1_name = "a.txt";
    char *res2_name = "b.txt";
    char *res3_name = "c.txt";

#ifndef SYS

    *data = fopen(data_name, "r");
    *res1 = fopen(res1_name, "w");
    *res2 = fopen(res2_name, "w");
    *res3 = fopen(res3_name, "w");
    if (*data == NULL || *res1 == NULL || *res2 == NULL || *res3 == NULL) {
        fprintf(stderr,"Cannot open file(s) %s\n", strerror(errno));
        exit(-1);
    }
#endif

#ifdef SYS

    *data = open(data_name, O_RDONLY | O_CREAT,S_IRUSR|S_IWUSR);
    *res1 = open(res1_name, O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR|S_IWUSR);
    *res2 = open(res2_name, O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR|S_IWUSR);
    *res3 = open(res3_name, O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR|S_IWUSR);
     if (*data < 0 || *res1 < 0 || *res2 < 0 || *res3 < 0){
        fprintf(stderr,"Cannot open file(s) %s\n", strerror(errno));
        exit(-1);
    }
#endif
}

void write_wrapper(MYFILE file, char *line) {
    WRITE;
}

int check_second_condition(int a) {
    // true if second to last digit is 0 or 7
    // it is assumed that single digits etc sarisfy condition
    a /= 10;
    return (a % 10 == 0) || (a % 10 == 7);
}

int check_if_sqrt(int a) {
    int b = a / 2 + 1;
    while (b * b > a) {
        b = b / 2;
    }
    while (b * b < a) {
        b++;
    }

    return b * b == a;
}

void process_numbers(char *data_name) {
    MYFILE data;
    MYFILE res1;
    MYFILE res2;
    MYFILE res3;
    open_files(&data, &res1, &res2, &res3, data_name);
    int empty = 0;
    int even = 0;
    char *line;

    while (1) {
        line = my_getline(data, &empty);
        if (empty || strlen(line) == 0) // case with new line at the end of file
            break;
        if (!is_positive_num(line)) {
            fprintf(stderr, "Invalid row in file - not a positive num: %s\n patr", line);
            CLOSE(data);
            CLOSE(res1);
            CLOSE(res2);
            CLOSE(res3);
            exit(-1);
        }

        int num = atoi(line);
        line[strlen(line)] = '\n'; //the line is allocated in such way that there is for sure one free space at the end
        if (num % 2 == 0)
            even++;
        if (check_second_condition(num) == 1)
            write_wrapper(res2, line);
        if (check_if_sqrt(num) == 1)
            write_wrapper(res3, line);

        free(line);
    }
    free(line);

    char res[40];
    sprintf(res, "Liczb parzystych jest %d\n", even);
    write_wrapper(res1, res);

    CLOSE(data);
    CLOSE(res1);
    CLOSE(res2);
    CLOSE(res3);
}

int main(int argc, char **argv) {

#ifdef TIME
    double user_time;
    double system_time;
    double real_time;
    time_wrapper wrapper;
    set_times(&wrapper);
#endif
    char *data_name;
    if (argc == 1) {
        data_name = "dane.txt";
    } else if (argc == 2) {
        data_name = argv[1];
    } else {
        fprintf(stderr, "Ivalid number of arguments\n");
        exit(-1);
    }
    process_numbers(data_name);

#ifdef TIME
    get_times(&wrapper, &real_time, &user_time, &system_time);
    printf("Real time: %f\n",real_time);
    printf("User time: %f\n",user_time);
    printf("System time: %f\n\n",system_time);
   

    FILE* res = fopen("pomiar_zad_3.txt", "a");
    if(res == NULL){
        fprintf(stderr, "Cannot open result file %s\n",strerror(errno));
    }
    else{
        if(argc == 2){
           fprintf(res, "Times for %s\n",argv[1]);
        }
        fprintf(res, "Real time: %f\n",real_time);
        fprintf(res, "User time: %f\n",user_time);
        fprintf(res, "System time: %f\n\n",system_time);
    }
    fclose(res);
#endif
}

