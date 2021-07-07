#ifndef SHARED 
#define SHARED

// makro sprawdzające wystąpienie błędu (zwrócenie -1)
#define CHECK_ERR(f) do {if ((f) == -1) {printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno)); exit(-1);}} while(0)

#define TABLE_SIZE 5
#define OVEN_SIZE 5


#define OVEN_LOCK_SEM "/oven_lock_sem"
#define OVEN_SIZE_SEM "/oven_size_sem"
#define TABLE_LOCK_SEM "/table_lock_sem"
#define TABLE_SIZE_SEM "/table_size_sem"
#define TABLE_EMPTY_SEM "/table_empty_sem"

#define SHARED_MEM "/shared_mem"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef struct restaurant {
	int oven[OVEN_SIZE];
	int oven_id;
	int oven_size;
	int table[TABLE_SIZE];
	int table_id;
	int table_size;
 
} restaurant;

void timestamp(char* res);
void validate_sem(sem_t* sem);
#endif
