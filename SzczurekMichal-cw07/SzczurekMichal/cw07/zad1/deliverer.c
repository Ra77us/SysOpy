
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h> 
#include <sys/ipc.h>
#include <sys/sem.h>
#include "shared.h"


int semaphores;
int shared_mem;
restaurant* pizzeria;


void clean(){
	CHECK_ERR(shmdt(pizzeria));
}

void sigint_handler(int signum){
	exit(0);
}

void setup(){
	key_t sem_key = ftok("./main.c", SEM_ID);
	CHECK_ERR(sem_key);
	semaphores = semget(sem_key, 4, 0);
	CHECK_ERR(semaphores);
	
	key_t mem_key = ftok("./main.c", MEM_ID);
	CHECK_ERR(mem_key);
	shared_mem = shmget(mem_key, sizeof(restaurant), 0);
	CHECK_ERR(shared_mem);
	
	pizzeria = shmat(shared_mem, NULL, 0);
	if (pizzeria == (void*)-1){
		printf("Error in file %s in line %d: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(-1);
	}
}

int take_from_table(){

	struct sembuf lock_table = {TABLE_LOCK_SEM, -1, 0};
	struct sembuf decrease_size_table = {TABLE_SIZE_SEM, 1, 0};
	struct sembuf remove_pizza_table = {TABLE_EMPTY_SEM, -1, 0};
	struct sembuf operations[3] = {lock_table, decrease_size_table, remove_pizza_table};
	CHECK_ERR(semop(semaphores, operations, 3));
	
	int pizza = pizzeria->table[(pizzeria->table_id - pizzeria->table_size + 1 + TABLE_SIZE) % TABLE_SIZE];
	pizzeria->table_size--;
	
	char time_str[30];
	timestamp(time_str);
	printf("(%d %s) Pobieram pizze: %d. Liczba pizz na stole: %d.\n", getpid(), time_str, pizza, pizzeria->table_size);
	
	struct sembuf unlock_table = {TABLE_LOCK_SEM, 1, 0};
	CHECK_ERR(semop(semaphores, &unlock_table, 1));
	
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
