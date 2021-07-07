#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <sys/times.h>
#include <errno.h>

#ifdef SYS
#define MYFILE int
#define READ read(source, &a, sizeof(char))
#define WRITE write(res, line, len)
#define CLOSE close
#define GO_BACK lseek(source, -1 * sizeof (char), SEEK_CUR)
#endif

#ifndef SYS
#define MYFILE FILE*
#define READ fread(&a, sizeof(char), 1, source)
#define WRITE fwrite(line, sizeof(char), len, res)
#define CLOSE fclose
#define GO_BACK fseek(source, -1 * sizeof (char), 1)
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

void write_wrapper(char *line, MYFILE res, int len) {
    WRITE;
}

void process_file(char *s, char *r, char *removed, char *added) {
    MYFILE source;
    MYFILE res;

#ifndef SYS
    source = fopen(s, "r");
    res = fopen(r, "w");
    if (source == NULL || res == NULL) {
        fprintf(stderr,"Cannot open file(s) %s\n", strerror(errno));
        exit(-1);
    }
#endif

#ifdef SYS
    source = open(s, O_RDONLY | O_CREAT,S_IRUSR|S_IWUSR);
    res = open(r, O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR|S_IWUSR);
        if (source < 0 ||res < 0){
        fprintf(stderr,"Cannot open file(s) %s\n", strerror(errno));
        exit(-1);
    }
#endif

    char a;
    int i = 0;
    while (READ) {
        if (a != removed[0]) {
            write_wrapper(&a, res, 1);
        } else { //potential replace
            i++;
            while (READ && i < strlen(removed) && a == removed[i]) {
                i++;
            }
            if (i == strlen(removed)) {
                write_wrapper(&added[0], res, strlen(added));
            } else {
                write_wrapper(&removed[0], res, i);
            }
            GO_BACK; // go 1 character back
            i = 0;
        }
    }

    CLOSE(source);
    CLOSE(res);
}

int main(int argc, char **argv) {

#ifdef TIME
    double user_time;
    double system_time;
    double real_time;
    time_wrapper wrapper;
    set_times(&wrapper);
#endif

    if (argc != 5) {
        fprintf(stderr, "Invalid number of arguments (4 required)\n");
        exit(-1);
    }
    process_file(argv[1], argv[2], argv[3], argv[4]);

#ifdef TIME
    get_times(&wrapper, &real_time, &user_time, &system_time);
    printf("Real time: %f\n",real_time);
    printf("User time: %f\n",user_time);
    printf("System time: %f\n\n",system_time);
   

    FILE* res = fopen("pomiar_zad_4.txt", "a");
    if(res == NULL){
        fprintf(stderr, "Cannot open result file %s\n",strerror(errno));
    }
    else{
        fprintf(res, "Replacing %s for %s in %s\n",argv[3],argv[4],argv[1]);
        fprintf(res, "Real time: %f\n",real_time);
        fprintf(res, "User time: %f\n",user_time);
        fprintf(res, "System time: %f\n\n",system_time);
    }
    fclose(res);
#endif
}

