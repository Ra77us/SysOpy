
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "shared.h"


int shared_mem;

sem_t* table_lock_sem;
sem_t* table_size_sem;
sem_t* table_empty_sem;
restaurant* pizzeria;

void sigint_handler(int signum){
	exit(0);
}

void clean(){
	CHECK_ERR(sem_close(table_lock_sem));
	CHECK_ERR(sem_close(table_size_sem));
	CHECK_ERR(sem_close(table_empty_sem));
	
	CHECK_ERR(munmap(pizzeria, sizeof(restaurant)));
}

void setup(){

	srand(getpid()<<4);
	
	shared_mem = shm_open(SHARED_MEM, O_RDWR, 0666);
	CHECK_ERR(shared_mem);
	
	pizzeria =  mmap(NULL, sizeof(restaurant), PROT_WRITE, MAP_SHARED, shared_mem, 0);
	if (pizzeria == (void *) -1){
		printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(-1);
	}
	
	table_lock_sem = sem_open(TABLE_LOCK_SEM,O_RDWR);
	validate_sem(table_lock_sem);
	
	table_size_sem = sem_open(TABLE_SIZE_SEM,O_RDWR);
	validate_sem(table_size_sem);
	
	table_empty_sem = sem_open(TABLE_EMPTY_SEM,O_RDWR);
	validate_sem(table_empty_sem);
}

int take_from_table(){
	
	CHECK_ERR(sem_wait(table_empty_sem));
	CHECK_ERR(sem_wait(table_lock_sem));
	CHECK_ERR(sem_post(table_size_sem));
	
	int pizza = pizzeria->table[(pizzeria->table_id - pizzeria->table_size + 1 + TABLE_SIZE) % TABLE_SIZE];
	pizzeria->table_size--;
	
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) Pobieram pizze: %d. Liczba pizz na stole: %d.\n", getpid(), time_str, pizza, pizzeria->table_size);
	
	CHECK_ERR(sem_post(table_lock_sem));
	
	return pizza;
}

void deliver(int pizza){
 	usleep((rand() % (10) + 40) * 100000);
   
	char time_str[30];
	timestamp(time_str);
    printf("(%d %s) Dostarczam pizze: %d.\n", getpid(), time_str, pizza);
    
    usleep((rand() % (10) + 40) * 100000);

}

void work(){
	while (1){
		int pizza = take_from_table();
		deliver(pizza);
	}
}

int main(){

	atexit(clean);
	signal(SIGINT, sigint_handler);
	setup();
	work();
}
