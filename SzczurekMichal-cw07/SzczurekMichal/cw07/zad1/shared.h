#ifndef SHARED 
#define SHARED

// makro sprawdzające wystąpienie błędu
#define CHECK_ERR(f) do {if ((f) == -1) {printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno)); exit(-1);}} while(0)

#define TABLE_SIZE 5
#define OVEN_SIZE 5


#define OVEN_LOCK_SEM 0
#define OVEN_SIZE_SEM 1
#define TABLE_LOCK_SEM 2
#define TABLE_SIZE_SEM 3
#define TABLE_EMPTY_SEM 4

#define MEM_ID 10
#define SEM_ID 11

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

typedef struct restaurant {
	int oven[OVEN_SIZE];
	int oven_id;
	int oven_size;
	int table[TABLE_SIZE];
	int table_id;
	int table_size;
 
} restaurant;

void timestamp(char* res);
#endif
