#include "shared.h"

void timestamp(char* res){
	struct timeval time;
    gettimeofday(&time, NULL);
    char tmp[20];
    strftime(tmp, 20, "%H:%M:%S", localtime(&time.tv_sec));
    sprintf(res, "%s:%03ld", tmp, time.tv_usec / 1000);
}

