#include "shared.h"

void timestamp(char* res){
	struct timeval time;
    gettimeofday(&time, NULL);
    char tmp[20];
    strftime(tmp, 20, "%H:%M:%S", localtime(&time.tv_sec));
    sprintf(res, "%s:%03ld", tmp, time.tv_usec / 1000);
}

void validate_sem(sem_t* sem){
	if(sem == SEM_FAILED){
		printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(-1);
	}
}  
